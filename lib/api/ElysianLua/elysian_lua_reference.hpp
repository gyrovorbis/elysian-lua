#ifndef ELYSIAN_LUA_REFERENCE_HPP
#define ELYSIAN_LUA_REFERENCE_HPP

extern "C" {
#   include <lua/lauxlib.h>
}

#include <ElysianLua/elysian_lua_thread_view.hpp>

namespace elysian::lua {

class ThreadView;

template<typename RefType>
class StatefulReference: public RefType {
public:
    StatefulReference(ThreadView* pThread);
    bool isValid(void) const;
    bool push(void) const;
    bool pull(void) const;
    bool fromStackIndex(int index);
    int makeStackIndex(void) const;

    ThreadView* getThread(void) const;

private:
    ThreadView* m_pThread = nullptr;

};

class StatelessRegistryReference {
public:
    bool isValid(const ThreadView* pThread) const;
    bool push(const ThreadView* pThread) const;
    bool pull(const ThreadView* pThread) const;
    bool fromStackIndex(const ThreadView* pThread, int index);
    int makeStackIndex(const ThreadView* pThread) const;

    constexpr static bool onStack(void);

private:
    int m_pRef = LUA_NOREF;

};

class StatelessStackReference {
public:
    bool isValid(const ThreadView* pThread) const;
    bool push(const ThreadView* pThread) const;
    bool pull(const ThreadView* pThread) const;
    bool fromStackIndex(const ThreadView* pThread, int index);
    int makeStackIndex(const ThreadView* pThread) const;

    int getIndex(void) const;

    constexpr static bool onStack(void);

protected:
    int m_index = 0;
};

class StatelessGlobalsTablePsuedoReference {
public:
    bool isValid(const ThreadView* pThread) const;
    bool push(const ThreadView* pThread) const; //getglobals
    bool pull(const ThreadView* pThread) const; //return false
    bool fromStackIndex(const ThreadView* pThread, int index); //
    int makeStackIndex(const ThreadView* pThread) const;
};


class StackReference: public StatefulReference<StatelessStackReference> {
public:
    int getAbsStackIndex(void) const;
};


template<typename RefType>

inline StatefulReference<RefType>::StatefulReference(ThreadView* pThread): m_pThread(pThread) {}
template<typename RefType>
inline ThreadView* StatefulReference<RefType>::getThread(void) const { return m_pThread; }
template<typename RefType>
inline bool StatefulReference<RefType>::isValid(void) const { return isValid(m_pThread); }
template<typename RefType>
inline bool StatefulReference<RefType>::push(void) const { return RefType::push(m_pThread); }
template<typename RefType>
inline bool StatefulReference<RefType>::pull(void) const { return pull(m_pThread); }
template<typename RefType>
inline bool StatefulReference<RefType>::fromStackIndex(int index) { return fromStackIndex(m_pThread, index); }
template<typename RefType>
inline int StatefulReference<RefType>::makeStackIndex(void) const { return makeStackIndex(m_pThread); }


inline bool StatelessGlobalsTablePsuedoReference::push(const ThreadView* pThread) const { pThread->pushGlobalsTable(); return true; }







}

#endif // ELYSIAN_LUA_REFERENCE_HPP
