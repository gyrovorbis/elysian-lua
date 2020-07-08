#ifndef ELYSIAN_LUA_REFERENCE_HPP
#define ELYSIAN_LUA_REFERENCE_HPP

#include "elysian_lua_thread_view_base.hpp"
#include "elysian_lua_traits.hpp"


extern "C" {
#   include <lua/lauxlib.h>
}

namespace elysian::lua {

class Variant;

struct StatelessRefBaseTag {};
struct RefStateBaseTag {};

template<typename CRTP>
class RegistryRefBase;

template<typename CRTP>
class StackRefBase;

#ifdef ELYSIAN_LUA_ENABLE_CONCEPTS

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

template<typename T>
concept StatefulObject =
        requires(T t) {
            { t.getThread() } -> same_as<const ThreadViewBase*>;
        }; 

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

#endif

template<typename RefType, typename StateType>
class StatefulRefBase:
        public RefType,
        public StateType
{
public:

    StatefulRefBase(const ThreadViewBase* pThread=nullptr) {
        StateType::setThread(pThread);
    }

    StatefulRefBase(const StatefulRefBase<RefType, StateType>& rhs):
        StateType(static_cast<const StateType&>(rhs))
    {
        RefType::copy(this->getThread(), static_cast<const RefType&>(rhs));
    }

    StatefulRefBase(StatefulRefBase<RefType, StateType>&& rhs):
        StateType(static_cast<StateType&&>(std::move(rhs)))
    {
        RefType::move(this->getThread(), static_cast<RefType&&>(std::move(rhs)));
        rhs.setThread(nullptr);
    }

    StatefulRefBase<RefType, StateType>&
    operator=(const StatefulRefBase<RefType, StateType>& rhs) {
        release(this->getThread());
        this->setThread(rhs.getThread());
        RefType::copy(this->getThread(), static_cast<const RefType&>(rhs));
        return *this;
    }

    StatefulRefBase<RefType, StateType>&
    operator=(StatefulRefBase<RefType, StateType>&& rhs) {
        release(this->getThread());
        this->setThread(rhs.getThread());
        RefType::move(this->getThread(), static_cast<RefType&&>(std::move(rhs)));
        rhs.setThread(nullptr);
        return *this;
    }

    template<typename R>
    requires ReadableReferenceable<R>
    bool operator==(const R& rhs) const {
        bool equal = false;
        lua_State* pLState = this->getThread()? this->getThread()->getState() : nullptr;
        lua_State* pRState = rhs.getThread()? rhs.getThread()->getState() : nullptr;

        if(pLState == pRState) {
            if(!pLState) {
                equal = true;
            } else {
                if constexpr(std::is_convertible_v<RefType, R>) {
                    equal = this->compare(this->getThread(), rhs);
                } else {
                    const int stackIndex1 = this->makeStackIndex();
                    const int stackIndex2 = rhs.makeStackIndex();
                    if(stackIndex1 && stackIndex2) {
                        equal = this->getThread()->compare(stackIndex1, stackIndex2, LUA_OPEQ);
                    }
                    rhs.doneWithStackIndex(stackIndex2);
                    this->doneWithStackIndex(stackIndex1);
                }
            }
        }
        return equal;
    }

    ~StatefulRefBase(void) { release(this->getThread()); }

    bool release(const ThreadViewBase* pThread) {
        assert(pThread == this->getThread());
        bool retVal = RefType::release(this->getThread());
        StateType::clear();
        return retVal;
    }

    bool isValid(void) const {
        return this->getThread() != nullptr
                && this->getThread()->isValid()
                && RefType::isValid();
    }

    bool fromStackIndex(const ThreadViewBase* pThread, int index) {
        lua_State* pState1 = this->getThread()? this->getThread()->getState() : nullptr;
        lua_State* pState2 = pThread? pThread->getState() : nullptr;

        assert(!(StateType::staticState() && StackReferenceable<RefType> && pState1 != pState2));
        //Cannot store static stack references from other threads!!!
        release(this->getThread());
        this->setThread(pThread);
        return RefType::fromStackIndex(pThread, index);
    }

    bool push(const ThreadViewBase* pThread) const {
        bool retVal = false;
        if(!isValid()) {
            pThread->pushNil();
            retVal = true;
        } else {
            if constexpr(!StackReferenceable<RefType>) {
                retVal = RefType::push(pThread);
            } else {
                RefType::push(this->getThread());
                if(this->getThread() != pThread) {
                    this->getThread()->xmove(pThread->getState(), 1);
                }
                retVal = true;
            }
        }
        return retVal;
    }

    bool pull(const ThreadViewBase* pThread) {
        release(this->getThread());
        this->setThread(pThread);
        return RefType::pull(pThread);
    }

    using RefType::makeStackIndex;

    int makeStackIndex(void) const {
        return RefType::makeStackIndex(this->getThread());
    }

    using RefType::doneWithStackIndex;

    bool doneWithStackIndex(int index) const {
        return RefType::doneWithStackIndex(this->getThread(), index);
    }
};



template<typename CRTP>
class RefStateBase: public RefStateBaseTag {
public:
    using RefStateBaseType = CRTP;
};

class StaticRefState {
public:
    static const ThreadViewBase* staticThread(void);
    const ThreadViewBase* getThread(void) const;
    bool setThread(const ThreadViewBase*) { return false; }
    void clear(void) {  }
    constexpr static bool staticState(void) { return true; }
};

class ExplicitRefState {
public:
    const ThreadViewBase* getThread(void) const { return m_pThread; }
    bool setThread(const ThreadViewBase* pThreadView) { m_pThread = pThreadView; return true; }
    void clear(void) { m_pThread = nullptr; }
    constexpr static bool staticState(void) { return false; }
private:
    const ThreadViewBase* m_pThread = nullptr;

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
};

template<typename CRTP>
class RegistryRefBase:
    public StatelessRefBase<CRTP>
{
public:
    RegistryRefBase(void) = default;

    template<typename R>
    requires (RegistryReferenceable<std::decay_t<R>> && !(std::is_same_v<std::decay_t<R>, Variant> && std::is_same_v<CRTP, Variant>))
    auto    operator=(R&& rhs) -> CRTP& {
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
        } else if constexpr(!StatefulObject<DR>) {
            setRegistryKey(rhs.getRegistryKey());
            if constexpr(std::is_rvalue_reference_v<decltype(rhs)>) {
                rhs.setRegistryKey(LUA_NOREF);
            }
        }
        return static_cast<CRTP&>(*this);
    }

    bool copy(const ThreadViewBase* pThread, const RegistryRefBase<CRTP>& rhs) {
        bool success = false;
        if(pThread && rhs.push(pThread)) {
            success = pull(pThread);
        } else {
            setRegistryKey(LUA_NOREF);
        }
        return success;
    }

    bool move(const ThreadViewBase* pThread, RegistryRefBase<CRTP>&& rhs) {
        setRegistryKey(rhs.getRegistryKey());
        rhs.setRegistryKey(LUA_NOREF);
        return true;
    }

    bool compare(const ThreadViewBase* pThread, const CRTP& rhs) {
        return getRegistryKey() == rhs.getRegistryKey();
    }

    bool    isNilRef(void) const {  return getRegistryKey() == LUA_REFNIL; }
    bool    isValid(void) const { return getRegistryKey() != LUA_NOREF; }

    bool pull(const ThreadViewBase* pThread) {
        assert(pThread);
        bool success = false;
        release(pThread);
        if(pThread) {
            setRegistryKey(pThread->ref());
            success = true;
        }
        return success;
    }

    bool fromStackIndex(const ThreadViewBase* pThread, int index) {
        assert(pThread);
        bool success = false;
        if(pThread) {
            pThread->pushValue(index);
            success = pull(pThread);
        }
        return success;
    }

    bool push(const ThreadViewBase* pThread) const {
        assert(pThread);
        bool success = false;
        if(pThread) {
            if(!isValid()) {
                pThread->pushNil();
                success = true;
            } else {
                int retVal = pThread->getTableRaw(LUA_REGISTRYINDEX, this->getRegistryKey());
                assert(retVal != LUA_TNONE);
                if(retVal != LUA_TNONE) {
                    success = true;
                }
            }
        }
        return success;
    }

    int     makeStackIndex(const ThreadViewBase* pThread) const {
        assert(pThread);
        int index = 0;
        if(push(pThread)) {
            index = pThread->toAbsStackIndex(-1);
        }
        return index;
    }
    bool    doneWithStackIndex(const ThreadViewBase* pThread, int index) const {
        if(index) {
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

class StatelessRegRef:
    public RegistryRefBase<StatelessRegRef>
{
public:
    StatelessRegRef(void) = default;
    StatelessRegRef(const StatelessRegRef& rhs) {
        setRegistryKey(rhs.getRegistryKey());
    }
    StatelessRegRef& operator=(const StatelessRegRef&) = delete;

    using RegistryRefBase<StatelessRegRef>::operator=;

    template<typename R>
    requires ReadableReferenceable<std::decay_t<R>>
    StatelessRegRef(R&& rhs) {
        using DR = std::decay_t<R>;

        if constexpr(RegistryReferenceable<DR>) {
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
            } else if constexpr(!StatefulObject<DR>) {
                setRegistryKey(rhs.getRegistryKey());
                if constexpr(std::is_rvalue_reference_v<decltype(rhs)>) {
                    rhs.setRegistryKey(LUA_NOREF);
                }
            }
        } else {
            rhs.push(rhs.getThread());
            pull(rhs.getThread());
        }
    }

    void    setRegistryKey(int key) { m_ref = key; }
    int     getRegistryKey(void) const { return m_ref; }

private:

    int m_ref = LUA_NOREF;
};


template<typename CRTP>
class StackRefBase:
    public StatelessRefBase<CRTP> {
public:
    StackRefBase(void) = default;

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

    bool compare(const ThreadViewBase*, const CRTP& rhs) {
        return getStackIndex() == rhs.getStackIndex();
    }

    bool isValid(void) const { return getStackIndex() != 0; }

    bool fromStackIndex(const ThreadViewBase* pThread, int index) {
        bool success = false;
        assert(pThread->isValidIndex(index));
        if(index) {
            setStackIndex(pThread->toAbsStackIndex(index));
            success = true;
        }
        return success;
    }
    int makeStackIndex(const ThreadViewBase* pThread) const { return getStackIndex(); }
    bool doneWithStackIndex(const ThreadViewBase*, int) const { return true; }

    bool pull(const ThreadViewBase* pThread) { return fromStackIndex(pThread, -1); }
    bool push(const ThreadViewBase* pThread) const {
        bool success = false;
        if(isValid()) {
            pThread->pushValue(getStackIndex());
            success = true;
        }
        return success;
    }

    bool release(const ThreadViewBase*) {
        setStackIndex(0);
        return true;
    }

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

class StatelessStackRef:
        public StackRefBase<StatelessStackRef>
{
public:
    StatelessStackRef(void) = default;
    StatelessStackRef(const StatelessStackRef& rhs) {
        setStackIndex(rhs.getStackIndex());
    }
    StatelessStackRef& operator=(const StatelessStackRef&) = delete;

    using StackRefBase<StatelessStackRef>::operator=;

    void setStackIndex(int index) { m_index = index; }
    int getStackIndex(void) const { return m_index; }

private:
    int m_index = 0;
};

template<typename CRTP>
class FixedRefBase:
    public StatelessRefBase<CRTP> {
public:

    int makeStackIndex(const ThreadViewBase* pThread) const {
        int index = 0;
        if(push(pThread)) {
            index = pThread->toAbsStackIndex(-1);
        }
        return index;
    }

    bool doneWithStackIndex(const ThreadViewBase* pThread, int index) const {
        bool removed = true;
        if(index) {
            pThread->remove(index);
            removed = true;
        }
        return removed;
    }

    bool compare(const ThreadViewBase*, const CRTP&) { return true; }

    bool release(const ThreadViewBase*) { return true; }

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

class StatelessGlobalsTableFixedRef:
    public FixedRefBase<StatelessGlobalsTableFixedRef>
{
public:

    bool push(const ThreadViewBase* pThread) const {
        if(pThread) {
            pThread->pushGlobalsTable();
            return true;
        } else {
            return false;
        }
    }

};

namespace stack_impl {

template<ReadableReferenceable R>
struct stack_pusher<R> {
    static int push(const ThreadViewBase* pBase, StackRecord& record, const R& ref) {
       return static_cast<bool>(ref.push(pBase));
    }
};

template<WritableReferenceable R>
struct stack_checker<R> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isValidIndex(index) && pBase->getType(-1) != LUA_TNONE;
    }
};

template<WritableReferenceable R>
struct stack_getter<R> {
    static R get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        R ref;
        ref.fromStackIndex(pBase, index);
        return ref;
    }
};

template<>
inline constexpr int stack_pull_pop_count<StatelessStackRef> = 0;

}

}

#endif // ELYSIAN_LUA_REFERENCE_HPP
