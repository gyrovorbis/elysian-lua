#ifndef ELYSIAN_LUA_STACK_MONITOR_HPP
#define ELYSIAN_LUA_STACK_MONITOR_HPP

#include "elysian_lua_forward_declarations.hpp"
#include "elysian_lua_stack_frame.hpp"

namespace elysian::lua {

class StackMonitor {
private:
    const ThreadViewBase* m_pThread;
    int m_beginTop = LUA_REGISTRYINDEX;
    int m_endTop = LUA_REGISTRYINDEX;
public:
    StackMonitor(const ThreadViewBase* pThread=nullptr);
    const ThreadViewBase* getThread(void) const;

    int getBeginTop(void) const;
    int getEndTop(void) const;
    int getCurrentDelta(void) const;
    int getEndDelta(void) const;
    bool isCurrentlyBalanced(void) const;
    bool wasEndBalanced(void) const;

    int begin(void);
    int end(void);
};

template<bool RAII>
class StackGuard:
        public StackMonitor {
private:
    CppExecutionContext m_cppCtx;
    int m_expectedDelta = 0;
public:
    StackGuard(const ThreadViewBase* pThread, int expectedDelta=0, CppExecutionContext=CppExecutionContext());
    ~StackGuard(void);

    int getExpectedDelta(void) const;
    const CppExecutionContext& getCppExecutionContext(void) const;
    int end(void);
};



inline StackMonitor::StackMonitor(const ThreadViewBase* pThread):
    m_pThread(pThread),
    m_beginTop(pThread->getTop())
{}
inline const ThreadViewBase* StackMonitor::getThread(void) const { return m_pThread; }
inline int StackMonitor::getBeginTop(void) const { return m_beginTop; }
inline int StackMonitor::getEndTop(void) const { return m_endTop; }
inline int StackMonitor::getCurrentDelta(void) const { return getThread()->getTop() - getBeginTop(); }
inline int StackMonitor::getEndDelta(void) const { return getEndTop() - getBeginTop(); }
inline bool StackMonitor::isCurrentlyBalanced(void) const { return getCurrentDelta() == 0; }
inline bool StackMonitor::wasEndBalanced(void) const { return getEndDelta() == 0; }
inline int StackMonitor::begin(void) { return m_beginTop = getThread()->getTop(); }
inline int StackMonitor::end(void) {
    assert(getBeginTop() != LUA_REGISTRYINDEX); //end called before begin!
    m_endTop = getThread()->getTop();
    return getEndDelta();
}

template<bool RAII>
inline StackGuard<RAII>::StackGuard(const ThreadViewBase* pThread, int expectedDelta, CppExecutionContext ctx):
    StackMonitor(pThread),
    m_expectedDelta(expectedDelta),
    m_cppCtx(std::move(ctx))
{
    if constexpr(RAII) begin();
}

template<bool RAII>
inline int StackGuard<RAII>::getExpectedDelta(void) const { return m_expectedDelta; }

template<bool RAII>
inline StackGuard<RAII>::~StackGuard(void) {
    if constexpr(RAII) end();
}

template<bool RAII>
inline const CppExecutionContext& StackGuard<RAII>::getCppExecutionContext(void) const { return m_cppCtx; }

template<bool RAII>
inline int StackGuard<RAII>::end(void) {
    const int delta = StackMonitor::end();
    const Thread* pThread = Thread::fromState(getThread()->getState());

    if(delta != getExpectedDelta()) {
        pThread->setCurrentCppExecutionContext(m_cppCtx);
#if ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_LUA_ERROR
        getThread()->error("INVALID STACK DELTA [Expected: %d, Actual: %d, Begin: %d, End: %d]", getExpectedDelta(), delta, getBeginTop(), getEndTop());
#elif ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_LUA_WARNING
        getThread()->warn("INVALID STACK DELTA [Expected: %d, Actual: %d, Begin: %d, End: %d]", getExpectedDelta(), delta, getBeginTop(), getEndTop());
#elif ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_ASSERT
    assert(false);
#endif
        pThread->syncCppCallerContexts();
    }
    return delta;
}

}

#endif // ELYSIAN_LUA_STACK_MONITOR_HPP
