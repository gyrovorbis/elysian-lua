#ifndef ELYSIAN_LUA_CALLABLE_HPP
#define ELYSIAN_LUA_CALLABLE_HPP

#include "elysian_lua_forward_declarations.hpp"
#include "elysian_lua_stack_frame.hpp"
#include "elysian_lua_function_result.hpp"
#include "elysian_lua_thread_view_base.hpp"
#include "elysian_lua_vm.hpp"

namespace elysian::lua {

template<typename CRTP>
class FunctionCallerBase {
protected:
    struct CallInfo {
        int argc = 0;
        int retc = 0;
        int funcIndex = 0;
    };

    auto callFunction(const ThreadViewBase* pThread, int argc, int retc) const {
        typename CRTP::CallInfo callInfo;
        updateInfoPreCall(pThread, &callInfo, argc, retc);
        static_cast<const CRTP*>(this)->call(pThread, &callInfo);

        updateInfoPostCall(pThread, &callInfo);
        return static_cast<const CRTP*>(this)->createFunctionResult(pThread, callInfo);
    }

    void updateInfoPreCall(const ThreadViewBase* pThread, CallInfo* pInfo, int argc, int retc) const {
        pInfo->argc = argc;
        pInfo->retc = retc;
        pInfo->funcIndex = pThread->getTop() - argc;
    }

    void updateInfoPostCall(const ThreadViewBase* pThread, CallInfo* pInfo) const {
        pInfo->retc = pThread->getTop() - pInfo->funcIndex + 1;
    }
};

class FunctionCaller:
        public FunctionCallerBase<FunctionCaller>
{
    friend class FunctionCallerBase<FunctionCaller>;

protected:
    void call(const ThreadViewBase* pThread, CallInfo* pInfo) const {
        pThread->call(pInfo->argc, pInfo->retc);
    }

    FunctionResult createFunctionResult(const ThreadViewBase* pThread, const CallInfo& info) const {
        return FunctionResult(pThread, info.funcIndex, info.retc);
    }
};

template<typename MsgHandlerState>
class ProtectedFunctionCaller:
        public FunctionCallerBase<ProtectedFunctionCaller<MsgHandlerState>>,
        public MsgHandlerState
{
    using CallerBase = FunctionCallerBase<ProtectedFunctionCaller<MsgHandlerState>>;
    friend CallerBase;
protected:

    struct CallInfo:
            public CallerBase::CallInfo {
        int errorCode = -1;
    };

    void call(const ThreadViewBase* pThread, CallInfo* pInfo) const {
        int handlerIndex = 0;
        if(MsgHandlerState::pushMessageHandler(pThread)) {
            handlerIndex = 1;
            pThread->insert(1);
        }

        pInfo->errorCode = pThread->pCall(pInfo->argc, pInfo->retc, handlerIndex);

        if(handlerIndex != 0) {
            pThread->remove(1);
        }
    }

    ProtectedFunctionResult createFunctionResult(const ThreadViewBase* pThread, const CallInfo& info) const {
        return ProtectedFunctionResult(pThread, info.errorCode, info.funcIndex, info.retc);
    }
};

template<typename FunctionType>
class ExplicitMessageHandlerState {
private:
    FunctionType m_funcRef;
protected:

    bool pushMessageHandler(const ThreadViewBase* pThread) const {
        return pThread->push(m_funcRef);
    }
public:

    template<typename F>
    void setMessageHandler(F&& func) {
        m_funcRef = std::forward<F>(func);
    }
};

class StaticMessageHandlerState {
protected:
    bool pushMessageHandler(const ThreadViewBase* pThread) const {
        bool success = false;
        if(pThread->push(LuaVM::getGlobalMessageHandler())) {
            if(pThread->isNil(-1)) {
                pThread->pop();
            } else success = true;
        }
        return success;
    }
};

template<typename CRTP,
         typename Caller>
class Callable: public Caller {
public:

    template<typename... Args>
    auto operator()(Args&&... args) const;

    template<typename... Args>
    auto contextualCall(CppExecutionContext ctx, Args&&... args) const;
};

template<typename CRTP, typename Caller>
template<typename... Args>
inline auto Callable<CRTP, Caller>::operator()(Args&&... args) const {
    const CRTP* pSelf = static_cast<const CRTP*>(this);
    const ThreadViewBase* pThread = pSelf->getThread();
    assert(pThread);

    const int oldTop = pThread->getTop();
    pSelf->pushFunction();
    (pThread->push(std::forward<Args>(args)), ...);
    const int argPushCount = pThread->getTop() - oldTop - 1;
    return Caller::callFunction(pThread, argPushCount, LUA_MULTRET);
}

template<typename CRTP, typename Caller>
template<typename... Args>
inline auto Callable<CRTP, Caller>::contextualCall(CppExecutionContext ctx, Args&&... args) const {
    const CRTP* pSelf = static_cast<const CRTP*>(this);
    const ThreadViewBase* pThread = pSelf->getThread();

    const int oldTop = pThread->getTop();
    pSelf->pushFunction();
    (pThread->push(std::forward<Args>(args)), ...);
    const int argPushCount = pThread->getTop() - oldTop - 1;

    pThread->setCurrentCppExecutionContext(std::move(ctx));

    auto retVal = Caller::callFunction(pThread, argPushCount, LUA_MULTRET);
    pThread->syncCppCallerContexts();

    return retVal;


}

}


#endif // ELYSIAN_LUA_CALLABLE_HPP
