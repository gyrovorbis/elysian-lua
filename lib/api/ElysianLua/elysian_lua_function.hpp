#ifndef ELYSIAN_LUA_FUNCTION_HPP
#define ELYSIAN_LUA_FUNCTION_HPP

#include <functional>

#include "elysian_lua_reference.hpp"
#include "elysian_lua_function_result.hpp"
#include "elysian_lua_callable.hpp"

namespace elysian::lua {

//template whether protected or not when invoking () overload?
template<typename Ref>
class FunctionBase:
        public Ref,
        public Callable<FunctionBase<Ref>,
                        ProtectedFunctionCaller<StaticMessageHandlerState>> {
public:
    FunctionBase(void) = default;
    FunctionBase(const ThreadViewBase* pThread) requires WritableReferenceable<Ref>;

    //template<typename T, typename Key>
    //FunctionBase(const TableProxy<T, Key>& proxy);

    template<typename Ref2>
    FunctionBase(const FunctionBase<Ref2>& rhs);

    template<typename Ref2>
    FunctionBase(FunctionBase<Ref2>&& rhs);

    bool isValid(void) const;

    //=== Callable CRTP Overriddes ===
    bool pushFunction(void) const;
    // =============================

    operator bool(void) const;
    //template<typename T, typename Key>
    //const FunctionBase<Ref>& operator=(const TableProxy<T, Key>& proxy);

    FunctionBase<Ref>& operator=(auto&& rhs);

    //template<typename Ref2>
    //const FunctionBase<Ref>& operator=(const FunctionBase<Ref2>& rhs);

    //template<typename Ref2>
    //const FunctionBase<Ref>& operator=(FunctionBase<Ref2>&& rhs);

    const FunctionBase<Ref>& operator=(std::nullptr_t);

    bool operator==(const auto& rhs) const;

    bool operator==(std::nullptr_t) const;

    bool operator!=(auto&& rhs) const;

    bool operator!=(std::nullptr_t) const;

    template<typename R, typename... Args>
    operator std::function<R(Args...)>() const;

protected:

    //set error handler callback

};



template<typename Ref>
inline FunctionBase<Ref>::FunctionBase(const ThreadViewBase* pThread)
requires WritableReferenceable<Ref>:
    Ref(pThread)
{}
/*
template<typename Ref>
template<typename T, typename Key>
inline FunctionBase<Ref>::FunctionBase(const TableProxy<T, Key>& proxy):
    Object<Ref>(proxy.getThread())
{
    *this = proxy;
}*/

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
    return Ref::isValid();
 //       getType() == LUA_TFUNCTION;
}

template<typename Ref>
inline bool FunctionBase<Ref>::pushFunction(void) const {
    return this->getThread()->push(*this);
}

template<typename Ref>
inline FunctionBase<Ref>::operator bool(void) const {
    return isValid();
}
/*
template<typename Ref>
template<typename T, typename Key>
inline const FunctionBase<Ref>& FunctionBase<Ref>::operator=(const TableProxy<T, Key>& proxy) {
    if(proxy.getThread()->push(proxy)) {
        proxy.getThread()->pull(*this);
    }
    return *this;
}*/

template<typename Ref>
template<typename Ref2>
inline FunctionBase<Ref>& FunctionBase<Ref>::operator=(Ref2&& rhs) {
    static_cast<Ref&>(*this) = std::forward<Ref2>(rhs);
    return *this;
}

#if 0
template<typename Ref>
template<typename Ref2>
inline const FunctionBase<Ref>& FunctionBase<Ref>::operator=(const FunctionBase<Ref2>& rhs) {
    if(rhs.isValid()) {
       //Let the reference decide how to handle it
       if constexpr(std::is_same_v<Ref, Ref2>) {
           this->m_ref.copy(rhs.getThread(), rhs.m_ref);
       } else { // Manually copy via stack
           if(this->getThread()->push(rhs)) {
                this->m_ref.pull(this->getThread());
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
           this->m_ref.copy(rhs.getThread(), std::move(rhs.m_ref));
       } else { // Manually copy via stack
           if(this->getThread()->push(rhs)) {
                this->m_ref.pull(this->getThread());
           } else {
               *this = nullptr;
           }
       }
    } else {
        *this = nullptr;
    }
    return *this;
}
#endif

template<typename Ref>
inline const FunctionBase<Ref>& FunctionBase<Ref>::operator=(std::nullptr_t) {
    this->m_ref.destroy(this->getThread());
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
inline bool FunctionBase<Ref>::operator==(const auto& rhs) const {
    return static_cast<const Ref&>(*this) == rhs;
#if 0
    bool equal = false;
    if(!isValid() && !rhs.isValid()) { //two empty function refs are equal
        equal = true;
    } else if(this->getThread() == rhs.getThread()) {
        if(this->getThread()->push(rhs)) {
            if(this->getThread()->push(*this)) {
                equal = this->getThread()->compare(-1, -2, LUA_OPEQ);
                this->getThread()->pop();
            }
            this->getThread()->pop();
        }
    }
    return equal;
#endif
}

template<typename Ref>
inline bool FunctionBase<Ref>::operator!=(auto&& rhs) const {
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

template<typename Ref>
struct stack_checker<FunctionBase<Ref>> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isFunction(index);
    }
};

}

}

#endif // ELYSIAN_LUA_FUNCTION_HPP
