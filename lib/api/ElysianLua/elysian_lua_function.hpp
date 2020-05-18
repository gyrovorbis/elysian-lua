#ifndef ELYSIAN_LUA_FUNCTION_HPP
#define ELYSIAN_LUA_FUNCTION_HPP

#include <functional>

#include "elysian_lua_object.hpp"

namespace elysian::lua {

//template whether protected or not when invoking () overload?
template<typename Ref>
class FunctionBase:
        public Object<Ref>,
        public Callable<FunctionBase<Ref>> {
public:
    FunctionBase(const ThreadViewBase* pThread=nullptr);

    template<typename T, typename Key>
    FunctionBase(const TableProxy<T, Key>& proxy);

    template<typename Ref2>
    FunctionBase(const FunctionBase<Ref2>& rhs);

    template<typename Ref2>
    FunctionBase(FunctionBase<Ref2>&& rhs);

    bool isValid(void) const;

    //=== Callable CRTP Overriddes ===
    void pushFunc(void) const;
    int validateFunc(void) const;
    // =============================

    operator bool(void) const;

    template<typename T, typename Key>
    const FunctionBase<Ref>& operator=(const TableProxy<T, Key>& proxy);

    template<typename Ref2>
    const FunctionBase<Ref>& operator=(const FunctionBase<Ref2>& rhs);

    template<typename Ref2>
    const FunctionBase<Ref>& operator=(FunctionBase<Ref2>&& rhs);

    const FunctionBase<Ref>& operator=(std::nullptr_t);

    template<typename R>
    bool operator==(R&& rhs) const;

    bool operator==(std::nullptr_t) const;

    template<typename R>
    bool operator!=(R&& rhs) const;

    bool operator!=(std::nullptr_t) const;

#if 0
    template<typename... Args>
    ProtectedFunctionResult operator()(Args&&... args) const;
#endif

    template<typename R, typename... Args>
    operator std::function<R(Args...)>() const;

protected:

    //set error handler callback

};

template<typename CRTP, typename Result>
template<typename... Args>
inline Result Callable<CRTP, Result>::operator()(Args&&... args) const {
    const CRTP* self = static_cast<const CRTP*>(this);
    const int oldTop = self->getThread()->getTop();
    const int tempStackSlots = (getTemporaryStackSlots(std::forward<Args>(args)) + ...);
    int argPushCount = 0;
    int errorCode = self->validateFunc();

    if(errorCode == LUA_OK) {
        self->pushFunc();
        (self->getThread()->push(std::forward<Args>(args)), ...);

        argPushCount = self->getThread()->getTop() - oldTop - 1;
        errorCode = self->getThread()->pCall(argPushCount, LUA_MULTRET);
    }
    return ProtectedFunctionResult(self->getThread(), errorCode, oldTop+1-tempStackSlots, self->getThread()->getTop()-oldTop, tempStackSlots);
}

template<typename CRTP, typename Result>
template<typename T>
inline int Callable<CRTP, Result>::getTemporaryStackSlots(T&& arg) {
    if constexpr(std::is_same_v<T&&, FunctionResult&&>
                 || std::is_same_v<T&&, ProtectedFunctionResult&&>)
    {
#if 0
        return arg.getReturnCount() + arg.getTempStackSlots();
#else
        return 0;
#endif
    } else {
        return 0;
    }
}


template<typename Ref>
inline FunctionBase<Ref>::FunctionBase(const ThreadViewBase* pThread):
    Object<Ref>(pThread)
{}

template<typename Ref>
template<typename T, typename Key>
inline FunctionBase<Ref>::FunctionBase(const TableProxy<T, Key>& proxy):
    Object<Ref>(proxy.getThread())
{
    *this = proxy;
}

template<typename Ref>
template<typename Ref2>
inline FunctionBase<Ref>::FunctionBase(const FunctionBase<Ref2>& rhs):
    Object<Ref>(rhs.getThread())
{
    *this = rhs;
}

template<typename Ref>
template<typename Ref2>
inline FunctionBase<Ref>::FunctionBase(FunctionBase<Ref2>&& rhs):
    Object<Ref>(rhs.getThread())
{
    *this = std::move(rhs);
}

template<typename Ref>
inline bool FunctionBase<Ref>::isValid(void) const {
    return getThread() && m_ref.isValid();
}

template<typename Ref>
inline void FunctionBase<Ref>::pushFunc(void) const {
    m_ref.push();
}

template<typename Ref>
inline int FunctionBase<Ref>::validateFunc(void) const {
    return isValid()? LUA_OK : LUA_ERRERR;
}

template<typename Ref>
inline FunctionBase<Ref>::operator bool(void) const {
    return isValid();
}

template<typename Ref>
template<typename T, typename Key>
inline const FunctionBase<Ref>& FunctionBase<Ref>::operator=(const TableProxy<T, Key>& proxy) {
    if(proxy.getThread()->push(proxy)) {
        proxy.getThread()->pull(*this);
    }
    return *this;
}

template<typename Ref>
template<typename Ref2>
inline const FunctionBase<Ref>& FunctionBase<Ref>::operator=(const FunctionBase<Ref2>& rhs) {
    if(rhs.isValid()) {
       //Let the reference decide how to handle it
       if constexpr(std::is_same_v<Ref, Ref2>) {
           m_ref.copy(rhs.getThread(), rhs.m_ref);
       } else { // Manually copy via stack
           if(getThread()->push(rhs)) {
                m_ref.pull(getThread());
           } else {
                *this = nullptr;
           }
       }
    } else {
        *this = nullptr;
    }
    return *this;
}

template<typename Ref>
template<typename Ref2>
inline const FunctionBase<Ref>& FunctionBase<Ref>::operator=(FunctionBase<Ref2>&& rhs) {
    if(rhs.isValid()) {
       //Let the reference decide how to handle it
       if constexpr(std::is_same_v<Ref, Ref2>) {
           m_ref.copy(rhs.getThread(), std::move(rhs.m_ref));
       } else { // Manually copy via stack
           if(getThread()->push(rhs)) {
                m_ref.pull(getThread());
           } else {
               *this = nullptr;
           }
       }
    } else {
        *this = nullptr;
    }
    return *this;
}

template<typename Ref>
inline const FunctionBase<Ref>& FunctionBase<Ref>::operator=(std::nullptr_t) {
    m_ref.destroy(getThread());
    return *this;
}

template<typename Ref>
inline bool FunctionBase<Ref>::operator==(std::nullptr_t) const {
    return !isValid();
}

template<typename Ref>
inline bool FunctionBase<Ref>::operator!=(std::nullptr_t) const {
    return isValid();
}

template<typename Ref>
template<typename R>
inline bool FunctionBase<Ref>::operator==(R&& rhs) const {
    bool equal = false;
    if(!isValid() && !rhs.isValid()) { //two empty function refs are equal
        equal = true;
    } else if(getThread() == rhs.getThread()) {
        if(getThread()->push(rhs)) {
            if(getThread()->push(*this)) {
                equal = getThread()->compare(-1, -2, LUA_OPEQ);
                getThread()->pop();
            }
            getThread()->pop();
        }
    }
    return equal;
}

template<typename Ref>
template<typename R>
inline bool FunctionBase<Ref>::operator!=(R&& rhs) const {
    return !(*this == rhs);
}
#if 0
template<typename Ref>
template<typename... Args>
inline ProtectedFunctionResult FunctionBase<Ref>::operator()(Args&&... args) const {
    const int oldTop = getThread()->getTop();
    m_ref.push();
    (getThread()->push(std::forward<Args>(args)), ...);
    /* This is so that we can chain functions by passing the results of one function call to another,
     * but the problem is that a temporary function return object would be immediately deleted after
     * returning from this function and would totally fuck the stack up. We have to take its deletion
     * into account early. */
    const int tempStackSlots = (getTemporaryStackSlots(std::forward<Args>(args)) + ...);
    const int argPushCount = getThread()->getTop() - oldTop - 1;
    const int errorCode = getThread()->pCall(argPushCount, LUA_MULTRET);
    return ProtectedFunctionResult(getThread(), errorCode, oldTop+1-tempStackSlots, getThread()->getTop()-oldTop, tempStackSlots);
}
#endif

template<typename Ref>
template<typename R, typename... Args>
inline FunctionBase<Ref>::operator std::function<R(Args...)>() const {
    return [&](Args... args) -> R {
        return *this(args...);
    };
}

namespace stack_impl {

struct function_stack_checker {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isFunction(index);
    }
};

template<>
struct stack_checker<Function>:
        public function_stack_checker {};

template<>
struct stack_getter<Function>:
        public object_stack_getter<Function> {};

template<>
struct stack_pusher<Function>:
        public object_stack_pusher<Function> {};

template<>
struct stack_checker<StackFunction>:
        public function_stack_checker {};

template<>
struct stack_getter<StackFunction>:
        public object_stack_getter<StackFunction> {};

template<>
struct stack_pusher<StackFunction>:
        public object_stack_pusher<StackFunction> {};



}



}

#endif // ELYSIAN_LUA_FUNCTION_HPP
