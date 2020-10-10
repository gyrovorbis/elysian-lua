#ifndef ELYSIAN_LUA_REFERENCE_HPP
#define ELYSIAN_LUA_REFERENCE_HPP

#include "elysian_lua_thread_state.hpp"
#include "elysian_lua_reference_base.hpp"

namespace elysian::lua {

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
            } else if constexpr(!ThreadStateful<DR>) {
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


template<typename RefType, typename StateType>
class StatefulRefBase:
        public RefType,
        public StateType
{
    static_assert(ReadableReferenceable<RefType>, "RefType must meet constraints of the ReadableReferenceable concept!");
    static_assert(ThreadStateful<StateType>, "StateType must meet constraints of the ThreadStateful concept!");

public:
    // Default constructor
    StatefulRefBase(void) = default;

    // Stateful thread constructor
    StatefulRefBase(const ThreadViewBase* pThread)
        requires(ModifiableThreadStateful<StateType>)
    {
        StateType::setThread(pThread);
    }

    // Copy constructor (REQUIRED)
    StatefulRefBase(const StatefulRefBase<RefType, StateType>& rhs) {
        copy(rhs);
    }

    // Generic copy constructor
    template<typename R2>
    StatefulRefBase(const R2& rhs) {
        copy(rhs);
    }

    // Generic move constructor
    template<typename RefType2>
    StatefulRefBase(RefType2&& rhs)
        requires std::is_rvalue_reference_v<decltype(rhs)>
    {
        move(std::move(rhs));
    }

    // Copy assignment operator
    template<typename R2>
    StatefulRefBase<RefType, StateType>& operator=(const R2& rhs) {
        release();
        copy(rhs);
        return *this;
    }

    //  Move assignment operator
    template<typename RefType2>
    StatefulRefBase<RefType, StateType>&
    operator=(RefType2&& rhs)
        requires std::is_rvalue_reference_v<decltype(rhs)>
    {
        release();
        move(std::forward<RefType2>(rhs));
        return *this;
    }

    // Generic equality comparison operator
    template<typename R>
        requires ReadableReferenceable<R>
    bool operator==(const R& rhs) const {
        bool equal = false;

        if(ThreadViewBase::compare(this->getThread(), rhs.getThread())) {
            if(!this->getThread()) {
                equal = true;
            } else {
                if constexpr(std::is_same_v<typename RefType::StatelessRefType, typename R::StatelessRefType> && !RegistryReferenceable<RefType>) {
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

    // Generic inequality comparison operator
    template<typename R>
        requires ReadableReferenceable<R>
    bool operator!=(const R& rhs) const {
        return !(*this == rhs);
    }

    // Generic copy logic
    template<ThreadStateful RefType2>
    void copy(const RefType2& rhs) {
        if constexpr(ModifiableThreadStateful<StateType>) {
            StateType::setThread(rhs.getThread());
        } else {
            assert(ThreadViewBase::compare(StateType::getThread(), rhs.getThread()));
        }
        RefType::copy(rhs.getThread(), rhs);
    }

    // Generic move logic
    template<ThreadStateful RefType2>
    void move(RefType2&& rhs)
        requires MoveCompatibleReferenceables<std::decay_t<RefType>, std::decay_t<RefType2>>
    {
        if constexpr(ModifiableThreadStateful<StateType>) {
            StateType::setThread(rhs.getThread());
        } else {
            assert(ThreadViewBase::compare(StateType::getThread(), rhs.getThread()));
        }
        RefType::move(rhs.getThread(), std::forward<RefType2>(rhs));

        if constexpr(ModifiableThreadStateful<RefType2>) {
            rhs.setThread(nullptr);
        }
    }

    // Generic move falling through to copy
    template<ThreadStateful RefType2>
    void move(RefType2&& rhs)
        requires (!MoveCompatibleReferenceables<std::decay_t<RefType>, std::decay_t<RefType2>>)
    {
        copy(rhs);
    }

    ~StatefulRefBase(void) { release(); }

    bool release(void) {
        bool retVal = RefType::release(this->getThread());
        if constexpr(ModifiableThreadStateful<StateType>) {
            StateType::setThread(nullptr);
        }
        return retVal;
    }

    bool isValid(void) const {
        return this->getThread() != nullptr
                && this->getThread()->isValid()
                && RefType::isValid();
    }

    //WRONG AS FUCK -- not handling threads properly!
    bool fromStackIndex(const ThreadViewBase* pThread, int index) {
        const auto* pThr = this->getThread();
        const lua_State* pState1 = pThr? pThr->getState() : nullptr;
        const lua_State* pState2 = pThread? pThread->getState() : nullptr;

        assert(!(!ModifiableThreadStateful<StateType> && StackReferenceable<RefType> && pState1 != pState2));
        //Cannot store static stack references from other threads!!!
        release();

        if constexpr(ModifiableThreadStateful<StateType>) {
            this->setThread(pThread);
        } else {
            assert(pState1 == pState2);
        }

        return RefType::fromStackIndex(pThread, index);
    }

    bool push(const ThreadViewBase* pThread) const {
        assert(pThread);

        bool retVal = false;
        if(!isValid()) {
            pThread->pushNil();
            retVal = true;
        } else {
            if constexpr(!StackReferenceable<RefType>) {
                retVal = RefType::push(pThread);
            } else {
                RefType::push(this->getThread());
                if(!ThreadViewBase::compare(this->getThread(), pThread)) {
                    this->getThread()->xmove(pThread->getState(), 1);
                }
                retVal = true;
            }
        }
        return retVal;
    }

    bool pull(const ThreadViewBase* pThread) requires WritableReferenceable<RefType> {
        release();
        if constexpr(ModifiableThreadStateful<StateType>) {
            StateType::setThread(pThread);
        } else {
            assert(this->getThread() == pThread);
        }

        return RefType::pull(pThread);
    }

    // Required by Referenceable concepts!
    using RefType::makeStackIndex;
    using RefType::doneWithStackIndex;

    int makeStackIndex(void) const {
        return RefType::makeStackIndex(this->getThread());
    }

    bool doneWithStackIndex(int index) const {
        return RefType::doneWithStackIndex(this->getThread(), index);
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
inline constinit int stack_pull_pop_count<StatelessStackRef> = 0;

}

}

#endif // ELYSIAN_LUA_REFERENCE_HPP
