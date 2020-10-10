#ifndef ELYSIAN_LUA_PROTECTED_BLOCK_HPP
#define ELYSIAN_LUA_PROTECTED_BLOCK_HPP

#include "elysian_lua_callable.hpp"
#include "elysian_lua_stack_frame.hpp"

namespace elysian::lua {

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

    int operator=(auto&& lambda);

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

inline int ProtectedBlock::operator=(auto&& func) {
    [[maybe_unused]] StackGuard stackGuard(getThread());
    if(getThread()->push(&cFuncWrapper<decltype(func)>)) {
        if(getThread()->push(reinterpret_cast<void*>(this))) {
            m_pLambda = reinterpret_cast<void*>(&func);
            getThread()->setCurrentCppExecutionContext(*this);
            auto result = callFunction(getThread(), 1, 0);
            getThread()->syncCppCallerContexts();
            m_errorCode = result.getErrorCode();
        } else getThread()->pop();
    }
    return getErrorCode();
}

template<typename F>
inline int ProtectedBlock::cFuncWrapper(lua_State* pState) {
    ThreadViewBase pThread = ThreadViewBase(pState);
    ProtectedBlock* pSelf = static_cast<ProtectedBlock*>(pThread.toUserdata(-1));
    std::decay_t<F>* pFunc = static_cast<std::decay_t<F>*>(pSelf->getLambda());
    (*pFunc)();
    return 0;
}

}

#endif // ELYSIAN_LUA_PROTECTED_BLOCK_HPP
