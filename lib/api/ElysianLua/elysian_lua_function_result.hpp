#ifndef ELYSIAN_LUA_FUNCTION_RESULT_HPP
#define ELYSIAN_LUA_FUNCTION_RESULT_HPP

#include "elysian_lua_proxy.hpp"

namespace elysian::lua {


class FunctionResult:
        public Proxy<FunctionResult>,
        public Callable<FunctionResult, FunctionResult>
{
public:
    FunctionResult(const ThreadViewBase* pThreadViewBase,
                   int startIndex, int count=-1, int tempStackSlots=0); //-1 => ranges from [startIndex : stackTop]
    FunctionResult(FunctionResult&& rhs);
    ~FunctionResult(void); //pop shit off of stack

    // ==== Callable CRTP Overrides ====
    void pushFunc(void) const;
    int validateFunc(void) const;
    // ================================

    int getReturnCount(void) const;
    int getFirstIndex(void) const;
    int getLastIndex(void) const;
    int getTempStackSlots(void) const;
    const ThreadViewBase* getThread(void) const;

    int push(const ThreadViewBase* pThread) const;

    template<typename R>
    bool operator==(const R& rhs) const;
    template<typename R>
    bool operator!=(const R& rhs) const;
    StackProxy operator[](int offset) const;

    template<typename T>
    auto get(int offset=0) const;

    template<typename... Args>
    auto get(int offset=0) const -> std::enable_if_t<(sizeof...(Args) > 1), std::tuple<Args...>>;
    // iterators

protected:
    const ThreadViewBase* m_pThread = nullptr;
    int m_stackIndex = 0;
    int m_retCount = 0;
    int m_tempStackSlots = 0;
};


class ProtectedFunctionResult:
        public FunctionResult,
        public Callable<ProtectedFunctionResult>
{
public:

    ProtectedFunctionResult(const ThreadViewBase* pThreadViewBase, int errorCode, int startIndex, int count=-1, int tempStackSlots=0); //-1 => ranges from [startIndex : stackTop]
    ProtectedFunctionResult(ProtectedFunctionResult&& rhs);

    using Callable<ProtectedFunctionResult>::operator();

    // ==== Callable CRTP Overrides ====
    int validateFunc(void) const;
    // ================================

    bool succeeded(void) const;
    int getErrorCode(void) const;
    const char* getErrorMessage(void) const;

private:
    int m_errorCode = -1;
};





inline FunctionResult::FunctionResult(const ThreadViewBase* pThreadViewBase, int startIndex, int count, int tempStackSlots):
    m_pThread(pThreadViewBase),
    m_stackIndex(pThreadViewBase->toAbsStackIndex(startIndex)),
    m_retCount(count),
    m_tempStackSlots(tempStackSlots)
{
    if(count == -1) {
        m_retCount = getThread()->getTop() - m_stackIndex;
    }
}

inline FunctionResult::FunctionResult(FunctionResult&& rhs):
    m_pThread(rhs.m_pThread),
    m_stackIndex(rhs.m_stackIndex),
    m_retCount(rhs.m_retCount)
{
    rhs.m_retCount = 0;
}

inline FunctionResult::~FunctionResult(void) {
    if(m_retCount) {
        assert(m_stackIndex + (m_retCount-1) <= getThread()->getTop());
        getThread()->remove(getFirstIndex(), getReturnCount());
    }
}
inline const ThreadViewBase* FunctionResult::getThread(void) const { return m_pThread; }
inline int FunctionResult::getReturnCount(void) const { return m_retCount; }
inline int FunctionResult::getFirstIndex(void) const { return m_stackIndex; }
inline int FunctionResult::getLastIndex(void) const { return m_stackIndex + (m_retCount-1); }
inline int FunctionResult::getTempStackSlots(void) const { return m_tempStackSlots; }

inline void FunctionResult::pushFunc(void) const {
    getThread()->pushValue(getFirstIndex());
}
inline int FunctionResult::validateFunc(void) const {
    return (getReturnCount() == 1 && getThread()->getType(getFirstIndex()) == LUA_TFUNCTION)?
                LUA_OK : LUA_ERRERR;
}

template<typename R>
inline bool FunctionResult::operator==(const R& rhs) const {
    R tempVal = *this;
    return (tempVal == rhs);
}

template<typename R>
inline bool FunctionResult::operator!=(const R& rhs) const {
    return !(*this == rhs);
}

inline int FunctionResult::push(const ThreadViewBase* pThread) const {
    for(int i = 0; i < getReturnCount(); ++i) {
        pThread->pushValue(getFirstIndex() + i);
    }
    return getReturnCount();
}

template<typename T>
inline auto FunctionResult::get(int offset) const {
    assert(getFirstIndex() + offset <= getThread()->getTop());
    return getThread()->toValue<T>(getFirstIndex() + offset);
}

template<typename... Args>
inline std::enable_if_t<(sizeof...(Args) > 1),
std::tuple<Args...>> FunctionResult::get(int offset) const {
                     assert(false);
    assert(getFirstIndex() + offset + sizeof...(Args) <= getThread()->getTop());

    std::tuple<Args...> retValues;
    const auto indices = std::index_sequence_for<Args...>;

    (std::get<indices>(retValues) = getThread()->toValue<decltype(std::get<indices>(retValues))>(getFirstIndex() + offset))...;
    return retValues;
}

inline ProtectedFunctionResult::ProtectedFunctionResult(const ThreadViewBase* pThreadViewBase, int errorCode, int startIndex, int count, int tempStackSlots):
    FunctionResult(pThreadViewBase, startIndex, count, tempStackSlots),
    m_errorCode(errorCode)
{
    //assert(succeeded() || !count);
}

inline ProtectedFunctionResult::ProtectedFunctionResult(ProtectedFunctionResult&& rhs):
    FunctionResult(std::move(rhs)),
    m_errorCode(rhs.m_errorCode)
{}

inline bool ProtectedFunctionResult::succeeded(void) const { return m_errorCode == LUA_OK; }
inline int ProtectedFunctionResult::getErrorCode(void) const { return m_errorCode; }
inline int ProtectedFunctionResult::validateFunc(void) const {
    int errorCode = FunctionResult::validateFunc();
    if(errorCode == LUA_OK) {
        if(!succeeded()) errorCode = LUA_ERRERR;
    }
    return errorCode;
}

inline const char* ProtectedFunctionResult::getErrorMessage() const {
    const char* msg = nullptr;
    if(!succeeded()) {
        msg = getThread()->toValue<const char*>(getFirstIndex());
    }
    return msg;
}


namespace stack_impl {

template<>
struct stack_pusher<FunctionResult>:
        public proxy_stack_pusher<FunctionResult>{};

template<>
struct stack_pusher<ProtectedFunctionResult>:
        public proxy_stack_pusher<ProtectedFunctionResult>{};

}


}

#endif // ELYSIAN_LUA_FUNCTION_RESULT_HPP
