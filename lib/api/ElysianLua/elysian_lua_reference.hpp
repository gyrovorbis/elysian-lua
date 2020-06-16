#ifndef ELYSIAN_LUA_REFERENCE_HPP
#define ELYSIAN_LUA_REFERENCE_HPP

#include "elysian_lua_thread_view_base.hpp"

extern "C" {
#   include <lua/lauxlib.h>
}


namespace elysian::lua {

template<typename CRTP>
class Referenceable {

private:
    template<typename T>
    bool assignThroughStack(const Referenceable<T>& rhs) const {
        static_assert(!CRTP::stackStorage() || T::stackStorage(), "Cannot assign a non-stack ref type to a stack ref type!");

        const int stackIndex = static_cast<const T&>(rhs).makeStackIndex();
        static_cast<CRTP*>(this)->fromStackIndex(stackIndex);
        static_cast<const T&>(rhs.doneWithStackIndex(stackIndex));
    }
public:
    Referenceable(void);

    template<typename T>
    Referenceable(const Referenceable<T>& rhs) {
        assignThroughStack(rhs);
    }

    template<typename T>
    Referenceable(Referenceable<T>&& rhs) {
        *this = std::move(rhs);
    }

    template<typename T>
    const CRTP& operator=(const Referenceable<T>& rhs) const {
        assignThroughStack(rhs);
        return *static_cast<CRTP*>(this);
    }

    template<typename T>
    const CRTP& operator=(Referenceable<T>&& rhs) const {
        if constexpr(CRTP::registryStorage() && T::registryStorage()) {
            static_cast<CRTP*>(this)->setRegistryKey(static_cast<T&&>(rhs)->getRegistryKey());
            static_cast<T&&>(rhs)->setRegistryKey(LUA_NOREF);
        } else {
            assignThroughStack(rhs);
        }
    }

    template<typename T>
    bool operator==(const Referenceable<T>& rhs) const;
};


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
        release();
        this->setThread(rhs.getThread());
        RefType::copy(this->getThread(), static_cast<const RefType&>(rhs));
        return *this;
    }

    StatefulRefBase<RefType, StateType>&
    operator=(StatefulRefBase<RefType, StateType>&& rhs) {
        release();
        this->setThread(rhs.getThread());
        RefType::move(this->getThread(), static_cast<RefType&&>(rhs));
        rhs.setThread(nullptr);
        return *this;
    }

    template<typename RefType2, typename StateType2>
    bool operator==(const StatefulRefBase<RefType2, StateType2>& rhs) const {
        bool equal = false;
        lua_State* pLState = this->getThread()? this->getThread()->getState() : nullptr;
        lua_State* pRState = rhs.getThread()? rhs.getThread()->getState() : nullptr;

        if(pLState == pRState) {
            if(!pLState) {
                equal = true;
            } else {
                if constexpr(std::is_same_v<RefType, RefType2>) {
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

    ~StatefulRefBase(void) { release(); }

    bool release(void) {
        bool retVal = RefType::release(this->getThread());
        StateType::clear();
        return retVal;
    }

    bool isValid(void) const {
        return this->getThread() != nullptr
                && this->getThread()->isValid()
                && RefType::isValid(this->getThread());
    }

    bool fromStackIndex(const ThreadViewBase* pThread, int index) {
        lua_State* pState1 = this->getThread()? this->getThread()->getState() : nullptr;
        lua_State* pState2 = pThread? pThread->getState() : nullptr;

        assert(!(StateType::staticState() && RefType::stackStorage() && pState1 != pState2));
        //Cannot store static stack references from other threads!!!
        release();
        this->setThread(pThread);
        return RefType::fromStackIndex(pThread, index);
    }

    bool push(const ThreadViewBase* pThread) const {
        bool retVal = false;
        if(!isValid()) {
            pThread->pushNil();
            retVal = true;
        } else {
            if constexpr(!RefType::stackStorage()) {
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
        release();
        this->setThread(pThread);
        return RefType::pull(pThread);
    }

    int makeStackIndex(void) const {
        return RefType::makeStackIndex(this->getThread());
    }

    bool doneWithStackIndex(int index) const {
        return RefType::doneWithStackIndex(this->getThread(), index);
    }

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



class StatelessRegistryReference {
public:

    bool copy(const ThreadViewBase* pThread, const StatelessRegistryReference& rhs);
    bool copy(const ThreadViewBase* pThread, StatelessRegistryReference&& rhs);
    bool move(const ThreadViewBase* pThread, StatelessRegistryReference&& rhs) {
        return copy(pThread, std::move(rhs));
    }

    bool destroy(const ThreadViewBase* pThread);
    bool release(const ThreadViewBase* pThread) { return destroy(pThread); }
    bool compare(const ThreadViewBase* pThread, const StatelessRegistryReference& rhs) const;

    bool isValid(const ThreadViewBase* pThread) const;
    bool push(const ThreadViewBase* pThread) const;
    bool pull(const ThreadViewBase* pThread);
    bool fromStackIndex(const ThreadViewBase* pThread, int index);
    int makeStackIndex(const ThreadViewBase* pThread) const;
    bool doneWithStackIndex(const ThreadViewBase* pThread, int index) const;

    int getRegistryKey(void) const;
    void setRegistryKey(int key);

    constexpr static bool stackStorage(void) { return false; }
    constexpr static bool registryStorage(void) { return true; }

private:
    int m_ref = LUA_NOREF;
};

class StatelessStackReference {
public:
    bool isValid(const ThreadViewBase* pThread) const;
    bool push(const ThreadViewBase* pThread) const;
    bool pull(const ThreadViewBase* pThread);
    bool fromStackIndex(const ThreadViewBase* pThread, int index);
    int makeStackIndex(const ThreadViewBase* pThread) const;
    bool doneWithStackIndex(const ThreadViewBase* pThread, int index) const;

    bool copy(const ThreadViewBase* pThread, const StatelessStackReference& rhs);
    bool move(const ThreadViewBase* pThread, StatelessStackReference&& rhs) {
        return copy(pThread, std::move(rhs));
    }
    bool release(const ThreadViewBase* pThread) {
        return destroy(pThread);
    }
    bool destroy(const ThreadViewBase* pThread);
    bool compare(const ThreadViewBase* pThread, const StatelessStackReference& rhs) const;

    int getStackIndex(void) const;
    void setStackIndex(int index);
    constexpr static bool stackStorage(void) { return true; }
    constexpr static bool registryStorage(void) { return false; }

protected:
    int m_index = 0;
};

class StatelessGlobalsTablePsuedoReference {
public:
    bool copy(const ThreadViewBase* pThread, const StatelessGlobalsTablePsuedoReference& other);
    bool move(const ThreadViewBase* pThread, const StatelessGlobalsTablePsuedoReference&& other);
    bool destroy(const ThreadViewBase* pThread);
    bool compare(const ThreadViewBase* pThread, const StatelessGlobalsTablePsuedoReference& rhs) const;

    bool release(const ThreadViewBase* pThread) { return true; }

    bool isValid(const ThreadViewBase* pThread) const;
    bool push(const ThreadViewBase* pThread) const; //getglobals
    bool pull(const ThreadViewBase* pThread) const; //return false
    bool fromStackIndex(const ThreadViewBase* pThread, int index); //
    int makeStackIndex(const ThreadViewBase* pThread) const;
    bool doneWithStackIndex(const ThreadViewBase* pThread, int index) const;

    constexpr static bool stackStorage(void) { return false; }
    constexpr static bool registryStorage(void) { return false; }
};

inline bool StatelessRegistryReference::compare(const ThreadViewBase* pThread, const StatelessRegistryReference& rhs) const {
    bool equal = false;
    if(m_ref == rhs.m_ref) {
        equal = true;
    } else {
        const int index1 = makeStackIndex(pThread);
        const int index2 = rhs.makeStackIndex(pThread);
        if(index1 && index2) {
            equal = pThread->compare(index1, index2, LUA_OPEQ);
        }
        rhs.doneWithStackIndex(pThread, index2);
        doneWithStackIndex(pThread, index1);
    }
    return equal;
}

inline int StatelessRegistryReference::makeStackIndex(const ThreadViewBase* pThread) const {
    int index = 0;
    if(push(pThread)) {
        index = pThread->toAbsStackIndex(-1);
    }
    return index;
}

inline bool StatelessRegistryReference::doneWithStackIndex(const ThreadViewBase* pThread, int index) const {
    if(index) pThread->remove(index); return true;
}

inline int StatelessRegistryReference::getRegistryKey(void) const { return m_ref; }
inline void StatelessRegistryReference::setRegistryKey(int key) { m_ref = key; }

inline void StatelessStackReference::setStackIndex(int index) { m_index = index; }
inline int StatelessStackReference::getStackIndex(void) const { return m_index; }

inline bool StatelessStackReference::push(const ThreadViewBase* pThread) const { pThread->pushValue(getStackIndex()); return true; }

inline bool StatelessStackReference::pull(const ThreadViewBase* pThread) { return fromStackIndex(pThread, -1); }

inline bool StatelessStackReference::fromStackIndex(const ThreadViewBase* pThread, int index) { if(index) m_index = pThread->toAbsStackIndex(index); return true; }

inline int StatelessStackReference::makeStackIndex(const ThreadViewBase* pThread) const { return getStackIndex(); }

inline bool StatelessStackReference::doneWithStackIndex(const ThreadViewBase*, int) const { return true; }

inline bool StatelessStackReference::isValid(const ThreadViewBase* pThread) const {
    return m_index != 0 && pThread && pThread->isValidIndex(m_index) && pThread->isObject(m_index);
}

inline bool StatelessStackReference::destroy(const ThreadViewBase* pThread) {
    (void)pThread;
    m_index = 0;
    return true;
}

inline bool StatelessStackReference::copy(const ThreadViewBase* pThread, const StatelessStackReference& rhs) {
    m_index = rhs.m_index;
    return true;
}

inline bool StatelessStackReference::compare(const ThreadViewBase* pThread, const StatelessStackReference& rhs) const {
    bool equal = false;
    // Quick, dumbass stack index comparison
    if(getStackIndex() == rhs.getStackIndex()) {
        equal = true;
    } else {
        if(getStackIndex() != 0 && rhs.getStackIndex() != 0) {
            equal = pThread->compare(getStackIndex(), rhs.getStackIndex(), LUA_OPEQ);
        }
    }

    return equal;
}

inline bool StatelessGlobalsTablePsuedoReference::copy(const ThreadViewBase* pThread, const StatelessGlobalsTablePsuedoReference& other) {
    return true;
}

inline bool StatelessGlobalsTablePsuedoReference::compare(const ThreadViewBase* pThread, const StatelessGlobalsTablePsuedoReference& rhs) const {
    return true;
}

inline bool StatelessGlobalsTablePsuedoReference::isValid(const ThreadViewBase* pThread) const {
    return pThread && pThread->isValid();
}

inline int StatelessGlobalsTablePsuedoReference::makeStackIndex(const ThreadViewBase* pThread) const {
    int index = 0;
    if(push(pThread)) {
        index = pThread->toAbsStackIndex(-1);
    }
    return index;
}

inline bool StatelessGlobalsTablePsuedoReference::doneWithStackIndex(const ThreadViewBase* pThread, int index) const {
    if(index) pThread->remove(index); return true;
}

inline bool StatelessGlobalsTablePsuedoReference::move(const ThreadViewBase* pThread, const StatelessGlobalsTablePsuedoReference&& other) { return true; }


namespace stack_impl {

struct reference_stack_checker {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isTable(index) || pBase->isUserdata(index) || pBase->isFunction(index);
    }
};

template<typename T>
struct reference_stack_getter {
    static T get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        T ref;
        ref.fromStackIndex(pBase, index);
        return ref;
    }
};

template<typename T>
struct reference_stack_pusher {
    static int push(const ThreadViewBase* pBase, StackRecord& record, const T& ref) {
       ref.push(pBase);
       return 1;
    }
};

template<>
constexpr int stack_pull_pop_count<StackTable> = 0;

template<>
struct stack_checker<StackRef>:
    public reference_stack_checker
{};

template<>
struct stack_getter<StackRef>:
    public reference_stack_getter<StackRef>
{};

template<>
struct stack_pusher<StackRef>:
    public reference_stack_pusher<StackRef>
{};

template<>
struct stack_checker<RegistryRef>:
    public reference_stack_checker
{};

template<>
struct stack_getter<RegistryRef>:
    public reference_stack_getter<RegistryRef>
{};

template<>
struct stack_pusher<RegistryRef>:
    public reference_stack_pusher<RegistryRef>
{};


template<>
struct stack_checker<StatelessRegistryReference>:
    public reference_stack_checker
{};

template<>
struct stack_getter<StatelessRegistryReference>:
    public reference_stack_getter<StatelessRegistryReference>
{};

template<>
struct stack_pusher<StatelessRegistryReference>:
    public reference_stack_pusher<StatelessRegistryReference>
{};

template<>
struct stack_pusher<GlobalsTablePsuedoRef>:
    public reference_stack_pusher<GlobalsTablePsuedoRef>
{};

}
}

#endif // ELYSIAN_LUA_REFERENCE_HPP
