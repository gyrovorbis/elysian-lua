#ifndef ELYSIAN_LUA_STACK_FRAME_HPP
#define ELYSIAN_LUA_STACK_FRAME_HPP

#include "elysian_lua_callable.hpp"

namespace elysian::lua {

class ThreadViewBase;

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

class CppExecutionContext {
public:
    CppExecutionContext(const char* pFile=nullptr, const char* pFunc=nullptr, int line=-1, const char* pType=nullptr);

    operator bool() const;

    bool isEmpty(void) const;

    bool hasFileName(void) const;
    bool hasFunctionName(void) const;
    bool hasLineNumber(void) const;
    bool hasTypeName(void) const;

    const char* getFileName(void) const;
    const char* getFunctionName(void) const;
    const char* getTypeName(void) const;
    int getLineNumber(void) const;

    void setFileName(const char* pName);
    void setFunctionName(const char* pName);
    void setTypeName(const char* pType);
    void setLineNumber(int line);

    void clear(void);

private:
    const char* m_pFile = nullptr;
    const char* m_pFunc = nullptr;
    const char* m_pType = nullptr;
    int m_line = -1;
    bool m_protected = false;
    bool m_coroutine = false;
};

template<bool RAII=true>
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

inline CppExecutionContext::CppExecutionContext(const char* pFile, const char* pFunc, int line, const char* pType):
    m_pFile(pFile),
    m_pFunc(pFunc),
    m_pType(pType),
    m_line(line)
{}
inline CppExecutionContext::operator bool() const { return !isEmpty(); }
inline const char* CppExecutionContext::getFileName(void) const { return m_pFile; }
inline const char* CppExecutionContext::getFunctionName(void) const { return m_pFunc; }
inline const char* CppExecutionContext::getTypeName(void) const { return m_pType; }
inline int CppExecutionContext::getLineNumber(void) const { return m_line; }
inline bool CppExecutionContext::hasFileName(void) const { return m_pFile; }
inline bool CppExecutionContext::hasFunctionName(void) const { return m_pFunc; }
inline bool CppExecutionContext::hasTypeName(void) const { return m_pType; }
inline bool CppExecutionContext::hasLineNumber(void) const { return m_line != -1; }
inline bool CppExecutionContext::isEmpty(void) const {
    return !hasFileName() && !hasFunctionName() && !hasLineNumber() && !hasTypeName();
}
inline void CppExecutionContext::setFileName(const char* pName) { m_pFile = pName; }
inline void CppExecutionContext::setFunctionName(const char* pName) { m_pFunc = pName; }
inline void CppExecutionContext::setTypeName(const char* pName) { m_pType = pName; }
inline void CppExecutionContext::setLineNumber(int line) { m_line = line; }
inline void CppExecutionContext::clear(void) {
    setFileName(nullptr); setFunctionName(nullptr); setLineNumber(-1); setTypeName(nullptr);
}

class ProtectedBlock:
    protected ProtectedFunctionCaller<StaticMessageHandlerState>,
    public CppExecutionContext
{
    friend ProtectedFunctionCaller<StaticMessageHandlerState>;
public:
    ProtectedBlock(const ThreadViewBase* pThread, CppExecutionContext ctx=CppExecutionContext());


    void* getLambda(void) const;
    int getErrorCode(void) const;
    const ThreadViewBase* getThread(void) const;

    template<typename F>
    int operator=(F&& lambda);

private:

    template<typename F>
    static int cFuncWrapper(lua_State* pState);

    void* m_pLambda = nullptr;
    const ThreadViewBase* m_pThread = nullptr;
    int m_errorCode = -1;

};

inline ProtectedBlock::ProtectedBlock(const ThreadViewBase* pThread, CppExecutionContext ctx):
    CppExecutionContext(std::move(ctx)),
    m_pThread(pThread)
{}

inline void* ProtectedBlock::getLambda(void) const { return m_pLambda; }
inline int ProtectedBlock::getErrorCode(void) const { return m_errorCode; }
inline const ThreadViewBase* ProtectedBlock::getThread(void) const { return m_pThread; }

template<typename F>
inline int ProtectedBlock::operator=(F&& func) {
    StackGuard stackGuard(getThread());
    (void)stackGuard;
    if(getThread()->push(&cFuncWrapper<F&&>)) {
        if(getThread()->push(reinterpret_cast<void*>(this))) {
            m_pLambda = reinterpret_cast<void*>(&func);
            Thread* pThd =  Thread::fromState(getThread()->getState());
            pThd->setCurrentCppExecutionContext(*this);
            auto result = callFunction(getThread(), 1, 0);
            pThd->syncCppCallerContexts();
            m_errorCode = result.getErrorCode();
        } else getThread()->pop();
    }
    return getErrorCode();
}

template<typename F>
inline int ProtectedBlock::cFuncWrapper(lua_State* pState) {
    ThreadViewBase* pThread = Thread::fromState(pState);
    ProtectedBlock* pSelf = static_cast<ProtectedBlock*>(pThread->toUserdata(-1));
    std::decay_t<F>* pFunc = static_cast<std::decay_t<F>*>(pSelf->getLambda());
    (*pFunc)();
    return 0;
}

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


#endif // ELYSIAN_LUA_STACK_FRAME_HPP
