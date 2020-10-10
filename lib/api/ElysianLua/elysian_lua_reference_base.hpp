#ifndef ELYSIAN_LUA_REFERENCE_BASE_HPP
#define ELYSIAN_LUA_REFERENCE_BASE_HPP

#include "elysian_lua_traits.hpp"
#include "elysian_lua_thread_view_base.hpp"

extern "C" {
#   include <lua/lauxlib.h>
}


namespace elysian::lua {

class Variant;

struct StatelessRefBaseTag {};

template<typename CRTP>
class RegistryRefBase;

template<typename CRTP>
class StackRefBase;

template<typename T>
concept StackPushable = requires(T t, const ThreadViewBase* pThread) {
    { pThread->push(t) } -> same_as<int>;
};

template<typename T>
concept StackPullable = requires(T t, const ThreadViewBase* pThread) {
    { pThread->pull(t) } -> same_as<bool>;
};

template<typename T>
concept StackCompatible = StackPushable<T> && StackPullable<T>;

template<typename R>
concept BasicReferenceable =
    std::is_base_of_v<StatelessRefBaseTag, R> &&
    requires(R r) {
        typename R::StatelessRefType;
        { r.isValid() } -> same_as<bool>;
    };

template<typename R>
concept ReadableReferenceable =
    BasicReferenceable<R> &&
    StackPushable<R> &&
    requires(const ThreadViewBase* pThread, R r) {
        { r.makeStackIndex(pThread) }           -> same_as<int>;
        { r.doneWithStackIndex(pThread, 0) }    -> same_as<bool>;
    };

template<typename R>
concept WritableReferenceable =
    BasicReferenceable<R> &&
    StackPullable<R> &&
    requires(const ThreadViewBase* pThread, R r) {
        { r.fromStackIndex(pThread, 0) }        -> same_as<bool>;
    };

template<typename R>
concept FixedReferenceable =
    ReadableReferenceable<R> &&
    !WritableReferenceable<R> &&
    std::is_default_constructible_v<R> &&
    std::is_copy_constructible_v<R> &&
    std::is_move_constructible_v<R>;

template<typename R>
concept Referenceable =
    ReadableReferenceable<R> &&
    WritableReferenceable<R> &&
    std::is_default_constructible_v<R>;

template<typename R>
concept RegistryReferenceable =
    Referenceable<R> &&
    std::is_base_of_v<RegistryRefBase<R>, R> &&
    requires(const ThreadViewBase* pThread, R r) {
        { r.getRegistryKey() }  -> same_as<int>;
        { r.setRegistryKey(LUA_NOREF) };
    };

template<typename R>
concept StackReferenceable =
    Referenceable<R> &&
    //std::is_base_of_v<StackRefBase<R>, R> &&
    requires(R r) {
        { r.getStackIndex() } -> same_as<int>;
        { r.setStackIndex(0) };
    };

template<typename Dst, typename Src>
concept MoveCompatibleReferenceables =
    WritableReferenceable<Dst> &&
    ReadableReferenceable<Src> &&
    requires(Src src) {
        { Dst::move(nullptr, std::move(src)) };
    };

template<typename CRTP>
class StatelessRefBase:
    public StatelessRefBaseTag
{
public:
    using StatelessRefType = CRTP;

    bool isNilRef(void) const { return false; }
    explicit operator bool(void) const { return static_cast<const CRTP*>(this)->isValid() && !static_cast<const CRTP*>(this)->isNilRef(); }
    bool isValid(void) const { return true; }

    bool copy(const ThreadViewBase*, const CRTP&) { return false; }
    bool move(const ThreadViewBase*, CRTP&&) { return false; }

protected:
    StatelessRefBase(void) = default;
};

template<typename CRTP>
class RegistryRefBase:
    public StatelessRefBase<CRTP>
{
public:

    template<typename R>
        requires (RegistryReferenceable<std::decay_t<R>> && !(std::is_same_v<std::decay_t<R>, Variant> && std::is_same_v<CRTP, Variant>))
    auto    operator=(R&& rhs) -> CRTP& {
        move(std::forward<R>(rhs));
        return static_cast<CRTP&>(*this);
    }

    template<typename R>
        requires (RegistryReferenceable<std::decay_t<R>> && !(std::is_same_v<std::decay_t<R>, Variant> && std::is_same_v<CRTP, Variant>))
    bool move(R&& rhs) {
        using DR = std::decay_t<R>;

        bool assigned = false;
        if constexpr(std::is_same_v<DR, Variant>) {
            if constexpr(std::is_rvalue_reference_v<decltype(rhs)>) {
                if(rhs.isRefType()) {
                    setRegistryKey(rhs.getRegistryKey());
                    rhs.setRegistryKey(LUA_NOREF);
                    assigned = true;
                }
            }

            if(!assigned) {
                rhs.push();
                pull(rhs.getThread());
            }
        } else if constexpr(!ThreadStateful<DR>) {
            setRegistryKey(rhs.getRegistryKey());
            if constexpr(std::is_rvalue_reference_v<decltype(rhs)>) {
                rhs.setRegistryKey(LUA_NOREF);
            }
        }
        return true;
    }

    template<ReadableReferenceable R2>
    bool copy(const ThreadViewBase* pThread, const R2& rhs) {
        bool success = false;

        [[likely]] if(pThread) {
            const int tempIndex = rhs.makeStackIndex(pThread);
            [[likely]] if(tempIndex) {
                if(fromStackIndex(pThread, tempIndex)) {
                    success = true;
                }
            }
            rhs.doneWithStackIndex(pThread, tempIndex);
        }

        [[unlikely]] if(!success) {
            setRegistryKey(LUA_NOREF);
        }

        return success;
    }

    bool move(const ThreadViewBase* pThread, CRTP&& rhs) {
        setRegistryKey(rhs.getRegistryKey());
        rhs.setRegistryKey(LUA_NOREF);
        return true;
    }

    bool compare(const ThreadViewBase* pThread, const CRTP& rhs) const {
        return getRegistryKey() == rhs.getRegistryKey();
    }

    bool    isNilRef(void) const {  return getRegistryKey() == LUA_REFNIL; }
    bool    isValid(void) const { return getRegistryKey() != LUA_NOREF; }

    bool pull(const ThreadViewBase* pThread) {
        assert(pThread);
        bool success = false;
        release(pThread);
        [[likely]] if(pThread) {
            setRegistryKey(pThread->ref());
            success = true;
        }
        return success;
    }

    bool fromStackIndex(const ThreadViewBase* pThread, int index) {
        assert(pThread);
        bool success = false;
        [[likely]] if(pThread) {
            pThread->pushValue(index);
            success = pull(pThread);
        }
        return success;
    }

    bool push(const ThreadViewBase* pThread) const {
        assert(pThread);
        bool success = false;
        [[likely]] if(pThread) {
            [[unlikely]] if(!isValid()) {
                pThread->pushNil();
                success = true;
            } else {
                int retVal = pThread->getTableRaw(LUA_REGISTRYINDEX, this->getRegistryKey());
                assert(retVal != LUA_TNONE);
                [[likely]] if(retVal != LUA_TNONE) {
                    success = true;
                }
            }
        }
        return success;
    }

    int     makeStackIndex(const ThreadViewBase* pThread) const {
        assert(pThread);
        int index = 0;
        [[likely]] if(push(pThread)) {
            index = pThread->toAbsStackIndex(-1);
        }
        return index;
    }
    bool    doneWithStackIndex(const ThreadViewBase* pThread, int index) const {
        [[likely]] if(index) {
            pThread->remove(index);
            return true;
        } else return false;
    }

    bool    release(const ThreadViewBase* pThread) {
        bool released = false;
        if(pThread && isValid()) {
            pThread->unref(getRegistryKey());
            setRegistryKey(LUA_NOREF);
            released = true;
        }
        return released;
    }

protected:
    RegistryRefBase(void) = default;

private:

    template<typename T = CRTP>
    requires requires (T b) {
        { b.getRegistryKey() } -> same_as<int>;
    }
    int getRegistryKey(void) const
    {
        return static_cast<const CRTP*>(this)->getRegistryKey();
    }

    template<typename T = CRTP>
    requires requires (T b) {
        { b.setRegistryKey(0) };
    }
    void setRegistryKey(int key)
    {
        static_cast<CRTP*>(this)->setRegistryKey(key);
    }

};

template<typename CRTP>
class StackRefBase:
    public StatelessRefBase<CRTP> {
public:

    template<typename R>
    requires (StackReferenceable<std::decay_t<R>>)
    auto    operator=(R&& rhs) -> CRTP& {
        copy(nullptr, std::forward<R>(rhs));
        return static_cast<CRTP&>(*this);
    }

    bool copy(const ThreadViewBase* pThread, const StackRefBase<CRTP>& rhs) {
        setStackIndex(rhs.getStackIndex());
        return true;
    }
    bool move(const ThreadViewBase* pThread, StackRefBase<CRTP>&& rhs) {
        return copy(pThread, rhs);
    }

    bool compare(const ThreadViewBase*, const CRTP& rhs) const {
        return getStackIndex() == rhs.getStackIndex();
    }

    bool isValid(void) const { return getStackIndex() != 0; }

    bool fromStackIndex(const ThreadViewBase* pThread, int index) {
        bool success = false;
        assert(pThread->isValidIndex(index));
        [[likely]] if(index) {
            setStackIndex(pThread->toAbsStackIndex(index));
            success = true;
        }
        return success;
    }
    int makeStackIndex(const ThreadViewBase* pThread) const { return getStackIndex(); }
    bool doneWithStackIndex(const ThreadViewBase*, int) const { return true; }

    bool pull(const ThreadViewBase* pThread) { return fromStackIndex(pThread, -1); }
    bool push(const ThreadViewBase* pThread) const {
        bool success = true;
        [[likely]] if(isValid()) {
            pThread->pushValue(getStackIndex());
            success = true;
        } else {
            pThread->pushNil();
        }
        return success;
    }

    bool release(const ThreadViewBase*) {
        setStackIndex(0);
        return true;
    }

protected:
    StackRefBase(void) = default;

private:
    template<typename T = CRTP>
    requires requires (T b) {
        { b.getStackIndex() } -> same_as<int>;
    }
    int getStackIndex(void) const
    {
        return static_cast<const CRTP*>(this)->getStackIndex();
    }

    template<typename T = CRTP>
    requires requires (T b) {
        { b.setStackIndex(0) };
    }
    void setStackIndex(int index)
    {
        static_cast<CRTP*>(this)->setStackIndex(index);
    }
};

template<typename CRTP>
class FixedRefBase:
    public StatelessRefBase<CRTP> {
public:

    int makeStackIndex(const ThreadViewBase* pThread) const {
        int index = 0;
        [[likely]] if(push(pThread)) {
            index = pThread->toAbsStackIndex(-1);
        }
        return index;
    }

    bool doneWithStackIndex(const ThreadViewBase* pThread, int index) const {
        bool removed = true;
        [[likely]] if(index) {
            pThread->remove(index);
            removed = true;
        }
        return removed;
    }

    bool compare(const ThreadViewBase*, const CRTP&) const { return true; }

    bool release(const ThreadViewBase*) { return true; }

protected:
    FixedRefBase(void) = default;

private:
    template<typename T = CRTP>
    requires requires (T b, const ThreadViewBase* pThread) {
        { b.push(pThread) } -> same_as<bool>;
    }
    bool push(const ThreadViewBase* pThread) const
    {
        return static_cast<const CRTP*>(this)->push(pThread);
    }

};

}


#endif // ELYSIAN_LUA_REFERENCE_BASE_HPP
