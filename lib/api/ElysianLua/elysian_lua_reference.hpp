#ifndef ELYSIAN_LUA_REFERENCE_HPP
#define ELYSIAN_LUA_REFERENCE_HPP

#include "elysian_lua_thread_view_base.hpp"

extern "C" {
#   include <lua/lauxlib.h>
}

namespace elysian::lua {

#if 1

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
        RefType::copy(getThread(), static_cast<const RefType&>(rhs));
    }

    StatefulRefBase(StatefulRefBase<RefType, StateType>&& rhs):
        StateType(static_cast<StateType&&>(std::move(rhs)))
    {
        RefType::move(getThread(), static_cast<RefType&&>(std::move(rhs)));
    }

    StatefulRefBase<RefType, StateType>&
    operator=(const StatefulRefBase<RefType, StateType>& rhs) {
        release();
        setThread(rhs.getThread());
        RefType::copy(getThread(), static_cast<const RefType&>(rhs));
        return *this;
    }

    StatefulRefBase<RefType, StateType>&
    operator=(StatefulRefBase<RefType, StateType>&& rhs) {
        release();
        setThread(rhs.getThread());
        RefType::move(getThread(), static_cast<RefType&&>(rhs));
        rhs.setThread(nullptr);
        return *this;
    }

    bool operator==(const StatefulRefBase<RefType, StateType>& rhs) const {
        return (getThread() == rhs.getThread() && compare(getThread(), rhs));
    }

    ~StatefulRefBase(void) { release(); }

    bool release(void) {
        bool retVal = RefType::release(getThread());
        StateType::clear();
        return retVal;
    }

    bool isValid(void) const {
        return getThread() != nullptr
                && getThread()->isValid()
                && RefType::isValid(getThread());
    }

    bool fromStackIndex(const ThreadViewBase* pThread, int index) {
        lua_State* pState1 = getThread()? getThread()->getState() : nullptr;
        lua_State* pState2 = pThread? pThread->getState() : nullptr;

        assert(!(StateType::staticState() && RefType::stackStorage() && pState1 != pState2));
        //Cannot store static stack references from other threads!!!
        release();
        setThread(pThread);
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
                RefType::push(getThread());
                if(getThread() != pThread) {
                    getThread()->xmove(pThread->getState(), 1);
                }
                retVal = true;
            }
        }
        return retVal;
    }

    bool pull(const ThreadViewBase* pThread) {
        release();
        setThread(pThread);
        return RefType::pull(pThread);
    }

    int makeStackIndex(void) const {
        return RefType::makeStackIndex(getThread());
    }

    bool doneWithStackIndex(int index) const {
        return RefType::doneWithStackIndex(getThread(), index);
    }

};


class StaticRefState {
public:
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


#endif


template<typename RefType>
class StatefulReference: public RefType {
public:
    using RefType::isValid;
    using RefType::push;

    StatefulReference(const ThreadViewBase* pThread);
    StatefulReference(const StatefulReference<RefType>& other);
    StatefulReference(StatefulReference<RefType>&& other);
    ~StatefulReference(void);

    const StatefulReference<RefType>& operator=(const StatefulReference<RefType>& other);
    const StatefulReference<RefType>& operator=(StatefulReference<RefType>&& other);

    bool operator==(const StatefulReference<RefType>& rhs) const;

    bool isValid(void) const;
    bool push(void) const;
    bool pull(void);
    bool pull(const ThreadViewBase* pThread);
    bool fromStackIndex(const ThreadViewBase* pThread, int index);
    int makeStackIndex(void) const;
    bool doneWithStackIndex(int index) const;
    bool destroy(const ThreadViewBase* pThread);
    bool release(const ThreadViewBase* pThread) {
        return destroy(pThread);
    }
    bool release(void) {
        return release(getThread());
    }

    const ThreadViewBase* getThread(void) const;
    constexpr static bool stackStorage(void);
    constexpr static bool onStack(void);

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

    constexpr static bool onStack(void);
    constexpr static bool stackStorage(void) { return false; }

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

    bool copy(const ThreadViewBase* pThread, StatelessStackReference&& rhs);
    bool copy(const ThreadViewBase* pThread, const StatelessStackReference& rhs);
    bool move(const ThreadViewBase* pThread, StatelessStackReference&& rhs) {
        return copy(pThread, std::move(rhs));
    }
    bool release(const ThreadViewBase* pThread) {
        return destroy(pThread);
    }
    bool destroy(const ThreadViewBase* pThread);
    bool compare(const ThreadViewBase* pThread, const StatelessStackReference& rhs) const;

    int getIndex(void) const;

    constexpr static bool onStack(void);
    constexpr static bool stackStorage(void) { return true; }

protected:
    int m_index = 0;
};

class StatelessGlobalsTablePsuedoReference {
public:
    bool copy(const ThreadViewBase* pThread, const StatelessGlobalsTablePsuedoReference& other);
    bool destroy(const ThreadViewBase* pThread);
    bool compare(const ThreadViewBase* pThread, const StatelessGlobalsTablePsuedoReference& rhs) const;

    bool isValid(const ThreadViewBase* pThread) const;
    bool push(const ThreadViewBase* pThread) const; //getglobals
    bool pull(const ThreadViewBase* pThread) const; //return false
    bool fromStackIndex(const ThreadViewBase* pThread, int index); //
    int makeStackIndex(const ThreadViewBase* pThread) const;
    bool doneWithStackIndex(const ThreadViewBase* pThread, int index) const;

    constexpr static bool onStack(void);
};


template<typename RefType>
inline StatefulReference<RefType>::StatefulReference(const ThreadViewBase* pThread): m_pThread(pThread) {}
template<typename RefType>
inline StatefulReference<RefType>::StatefulReference(StatefulReference<RefType>&& other) {
    *this = std::move(other);
}
template<typename RefType>
inline StatefulReference<RefType>::StatefulReference(const StatefulReference<RefType>& other) {
    *this = other;
}
template<typename RefType>
inline const StatefulReference<RefType>& StatefulReference<RefType>::operator=(const StatefulReference<RefType>& other) {
    m_pThread = other.m_pThread;
    RefType::copy(m_pThread, other);
    return *this;
}
template<typename RefType>
inline const StatefulReference<RefType>& StatefulReference<RefType>::operator=(StatefulReference<RefType>&& other) {
    m_pThread = other.m_pThread;
    RefType::copy(m_pThread, static_cast<RefType&&>(other));
    return *this;
}
template<typename RefType>
bool StatefulReference<RefType>::operator==(const StatefulReference<RefType>& rhs) const {
    return (m_pThread == rhs.m_pThread && compare(m_pThread, rhs));
}

template<typename RefType>
inline StatefulReference<RefType>::~StatefulReference(void) { RefType::destroy(m_pThread); }
template<typename RefType>
inline const ThreadViewBase* StatefulReference<RefType>::getThread(void) const { return m_pThread; }
template<typename RefType>
inline bool StatefulReference<RefType>::isValid(void) const { return RefType::isValid(m_pThread); }
template<typename RefType>
inline bool StatefulReference<RefType>::push(void) const { return RefType::push(m_pThread); }
template<typename RefType>
inline bool StatefulReference<RefType>::pull(void) { return RefType::pull(m_pThread); }
template<typename RefType>
inline bool StatefulReference<RefType>::pull(const ThreadViewBase* pThread) {
    destroy(m_pThread);
    m_pThread = pThread;
    return pull();
}

template<typename RefType>
inline bool StatefulReference<RefType>::fromStackIndex(const ThreadViewBase* pThread, int index) {
    destroy(m_pThread); //Should we do this at this layer?
    m_pThread = pThread;
    return RefType::fromStackIndex(m_pThread, index);
}
template<typename RefType>
inline int StatefulReference<RefType>::makeStackIndex(void) const { return RefType::makeStackIndex(m_pThread); }
template<typename RefType>
inline bool StatefulReference<RefType>::doneWithStackIndex(int index) const { return RefType::doneWithStackIndex(m_pThread, index); }
template<typename RefType>
inline bool StatefulReference<RefType>::destroy(const ThreadViewBase* pThread) {
    bool retVal = RefType::destroy(pThread);
    m_pThread = nullptr;
    return retVal;
}
template<typename RefType>
inline constexpr bool StatefulReference<RefType>::onStack(void) { return RefType::onStack(); }

inline constexpr bool StatelessRegistryReference::onStack(void) { return false; }

inline bool StatelessRegistryReference::compare(const ThreadViewBase* pThread, const StatelessRegistryReference& rhs) const {
    return m_ref == rhs.m_ref;
}

inline int StatelessRegistryReference::makeStackIndex(const ThreadViewBase* pThread) const {
    int index = 0;
    if(push(pThread)) {
        index = pThread->toAbsStackIndex(-1);
    }
    return index;
}

inline bool StatelessRegistryReference::doneWithStackIndex(const ThreadViewBase* pThread, int index) const {
    pThread->remove(index); return true;
}

inline int StatelessStackReference::getIndex(void) const { return m_index; }

inline bool StatelessStackReference::push(const ThreadViewBase* pThread) const { pThread->pushValue(getIndex()); return true; }

inline bool StatelessStackReference::pull(const ThreadViewBase* pThread) { return fromStackIndex(pThread, -1); }

inline bool StatelessStackReference::fromStackIndex(const ThreadViewBase* pThread, int index) { m_index = pThread->toAbsStackIndex(index); return true; }

inline int StatelessStackReference::makeStackIndex(const ThreadViewBase* pThread) const { return getIndex(); }

inline bool StatelessStackReference::doneWithStackIndex(const ThreadViewBase*, int) const { return true; }

inline bool StatelessStackReference::isValid(const ThreadViewBase* pThread) const {
    return m_index != 0 && pThread && pThread->isValidIndex(m_index) && pThread->isObject(m_index);
}

inline bool StatelessStackReference::destroy(const ThreadViewBase* pThread) {
    (void)pThread;
    m_index = 0;
    return true;
}

inline bool StatelessStackReference::copy(const ThreadViewBase* pThread, StatelessStackReference&& rhs) {
    m_index = rhs.m_index;
    return true;
}

inline bool StatelessStackReference::copy(const ThreadViewBase* pThread, const StatelessStackReference& rhs) {
    m_index = rhs.m_index;
    return true;
}

inline bool StatelessStackReference::compare(const ThreadViewBase* pThread, const StatelessStackReference& rhs) const {
    bool equal = false;
    // Quick, dumbass stack index comparison
    if(getIndex() == rhs.getIndex()) {
        equal = true;
    } else {
        equal = pThread->compare(getIndex(), rhs.getIndex(), LUA_OPEQ);
    }

    return equal;
}

inline constexpr bool StatelessStackReference::onStack(void) { return true; }

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
    pThread->remove(index); return true;
}

inline constexpr bool StatelessGlobalsTablePsuedoReference::onStack(void) { return false; }

namespace stack_impl {

struct reference_stack_checker {
    static bool check(ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isTable(index) || pBase->isUserdata(index) || pBase->isFunction(index);
    }
};

template<typename T>
struct reference_stack_getter {
    static T get(ThreadViewBase* pBase, StackRecord& record, int index) {
        T ref;
        ref.fromStackIndex(pBase, index);
        return ref;
    }
};

template<typename T>
struct reference_stack_pusher {
    static int push(ThreadViewBase* pBase, StackRecord& record, const T& ref) {
       ref.push(pBase);
       return 1;
    }
};

template<>
int stack_pull_pop_count<StackTable> = 0;

template<>
struct stack_checker<StackReference>:
    public reference_stack_checker
{};

template<>
struct stack_getter<StackReference>:
    public reference_stack_getter<StackReference>
{};

template<>
struct stack_pusher<StackReference>:
    public reference_stack_pusher<StackReference>
{};

template<>
struct stack_checker<RegistryReference>:
    public reference_stack_checker
{};

template<>
struct stack_getter<RegistryReference>:
    public reference_stack_getter<RegistryReference>
{};

template<>
struct stack_pusher<RegistryReference>:
    public reference_stack_pusher<RegistryReference>
{};


template<>
struct stack_checker<StatelessRegistryReference>:
    public reference_stack_checker
{};

template<>
struct stack_getter<StatelessRegistryReference>:
    public reference_stack_getter<RegistryReference>
{};

template<>
struct stack_pusher<StatelessRegistryReference>:
    public reference_stack_pusher<RegistryReference>
{};

template<>
struct stack_pusher<GlobalsTablePsuedoReference>:
    public reference_stack_pusher<GlobalsTablePsuedoReference>
{};

}
}

#endif // ELYSIAN_LUA_REFERENCE_HPP
