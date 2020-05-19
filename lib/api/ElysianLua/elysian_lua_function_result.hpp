#ifndef ELYSIAN_LUA_FUNCTION_RESULT_HPP
#define ELYSIAN_LUA_FUNCTION_RESULT_HPP

#include "elysian_lua_proxy.hpp"

namespace elysian::lua {

template<typename CRTP>
class FunctionResultBase: public Proxy<CRTP> {
public:
    FunctionResultBase(const ThreadViewBase* pThreadViewBase,
                   int startIndex, int count=-1, int tempStackSlots=0); //-1 => ranges from [startIndex : stackTop]
    FunctionResultBase(void) = delete;
    FunctionResultBase(const FunctionResultBase<CRTP>& rhs) = delete;
    FunctionResultBase(FunctionResultBase<CRTP>&& rhs);
    ~FunctionResultBase(void); //pop shit off of stack

    bool pushFunction(void) const;
    int validateFunc(void) const;

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

class FunctionResult:
    public FunctionResultBase<FunctionResult>
{
public:
    FunctionResult(const ThreadViewBase* pThreadViewBase,
                   int startIndex, int count=-1, int tempStackSlots=0); //-1 => ranges from [startIndex : stackTop]
    FunctionResult(FunctionResultBase&& rhs);


};

inline FunctionResult::FunctionResult(const ThreadViewBase* pThreadViewBase,
               int startIndex, int count, int tempStackSlots):
    FunctionResultBase<FunctionResult> (pThreadViewBase, startIndex, tempStackSlots)
{}

inline FunctionResult::FunctionResult(FunctionResultBase&& rhs):
    FunctionResultBase<FunctionResult> (std::move(rhs))
{}


class ProtectedFunctionResult:
        public FunctionResultBase<ProtectedFunctionResult>
        //public Callable<ProtectedFunctionResult>*/
{
public:

    ProtectedFunctionResult(const ThreadViewBase* pThreadViewBase, int errorCode, int startIndex, int count=-1, int tempStackSlots=0); //-1 => ranges from [startIndex : stackTop]
    ProtectedFunctionResult(ProtectedFunctionResult&& rhs);
    ProtectedFunctionResult(const ProtectedFunctionResult& rhs) = delete;
    ProtectedFunctionResult(void) = delete;

    //using Callable<ProtectedFunctionResult>::operator();

    // ==== Callable CRTP Overrides ====
    bool pushFunction(void) const;
    // ================================

    bool succeeded(void) const;
    int getErrorCode(void) const;
    const char* getErrorMessage(void) const;

private:
    int m_errorCode = -1;
};




template<typename CRTP>
inline FunctionResultBase<CRTP>::FunctionResultBase(const ThreadViewBase* pThreadViewBase, int startIndex, int count, int tempStackSlots):
    m_pThread(pThreadViewBase),
    m_stackIndex(pThreadViewBase->toAbsStackIndex(startIndex)),
    m_retCount(count),
    m_tempStackSlots(tempStackSlots)
{
    if(count == -1) {
        m_retCount = getThread()->getTop() - m_stackIndex;
    }
}

template<typename CRTP>
inline FunctionResultBase<CRTP>::FunctionResultBase(FunctionResultBase&& rhs):
    m_pThread(rhs.m_pThread),
    m_stackIndex(rhs.m_stackIndex),
    m_retCount(rhs.m_retCount)
{
    rhs.m_retCount = 0;
}

template<typename CRTP>
inline FunctionResultBase<CRTP>::~FunctionResultBase(void) {
    if(m_retCount) {
        int curTop = getThread()->getTop();
        assert(getLastIndex() <= getThread()->getTop());
        getThread()->remove(getFirstIndex(), getReturnCount());
    }
}
template<typename CRTP>
inline const ThreadViewBase* FunctionResultBase<CRTP>::getThread(void) const { return m_pThread; }
template<typename CRTP>
inline int FunctionResultBase<CRTP>::getReturnCount(void) const { return m_retCount; }
template<typename CRTP>
inline int FunctionResultBase<CRTP>::getFirstIndex(void) const { return m_stackIndex; }
template<typename CRTP>
inline int FunctionResultBase<CRTP>::getLastIndex(void) const { return m_stackIndex + (m_retCount-1); }
template<typename CRTP>
inline int FunctionResultBase<CRTP>::getTempStackSlots(void) const { return m_tempStackSlots; }

template<typename CRTP>
inline bool FunctionResultBase<CRTP>::pushFunction(void) const {
    getThread()->pushValue(getFirstIndex());
    return 1;
}

template<typename CRTP>
template<typename R>
inline bool FunctionResultBase<CRTP>::operator==(const R& rhs) const {
    R tempVal = *this;
    return (tempVal == rhs);
}

template<typename CRTP>
template<typename R>
inline bool FunctionResultBase<CRTP>::operator!=(const R& rhs) const {
    return !(*this == rhs);
}

template<typename CRTP>
inline int FunctionResultBase<CRTP>::push(const ThreadViewBase* pThread) const {
    for(int i = 0; i < getReturnCount(); ++i) {
        pThread->pushValue(getFirstIndex() + i);
    }
    return getReturnCount();
}

template<typename CRTP>
template<typename T>
inline auto FunctionResultBase<CRTP>::get(int offset) const {
    assert(getFirstIndex() + offset <= getThread()->getTop());
    return getThread()->toValue<T>(getFirstIndex() + offset);
}

template<typename CRTP>
template<typename... Args>
inline std::enable_if_t<(sizeof...(Args) > 1),
std::tuple<Args...>> FunctionResultBase<CRTP>::get(int offset) const {
                     assert(false);
    assert(getFirstIndex() + offset + sizeof...(Args) <= getThread()->getTop());

    std::tuple<Args...> retValues;
    const auto indices = std::index_sequence_for<Args...>;

    (std::get<indices>(retValues) = getThread()->toValue<decltype(std::get<indices>(retValues))>(getFirstIndex() + offset))...;
    return retValues;
}

inline ProtectedFunctionResult::ProtectedFunctionResult(const ThreadViewBase* pThreadViewBase, int errorCode, int startIndex, int count, int tempStackSlots):
    FunctionResultBase(pThreadViewBase, startIndex, count, tempStackSlots),
    m_errorCode(errorCode)
{
    //assert(succeeded() || !count);
}

inline ProtectedFunctionResult::ProtectedFunctionResult(ProtectedFunctionResult&& rhs):
    FunctionResultBase(std::move(rhs)),
    m_errorCode(rhs.m_errorCode)
{}

inline bool ProtectedFunctionResult::succeeded(void) const { return m_errorCode == LUA_OK; }
inline int ProtectedFunctionResult::getErrorCode(void) const { return m_errorCode; }
inline bool ProtectedFunctionResult::pushFunction(void) const {
    if(getErrorCode() == LUA_OK) {
        return FunctionResultBase::pushFunction();
    }
    return false;
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
