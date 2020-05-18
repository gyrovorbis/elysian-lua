#ifndef ELYSIAN_LUA_PROXY_HPP
#define ELYSIAN_LUA_PROXY_HPP

#include <tuple>
#include <ElysianLua/elysian_lua_traits.hpp>
#include "elysian_lua_function.hpp"

namespace elysian::lua {

class ThreadViewBase;

template<typename CRTP>
class Proxy {
public:

    template<typename T>
    operator T() const;

    int push(void) const;
    bool isValid(void) const;

   // template<typename T>
   // operator T&() const;

    ThreadViewBase* getThread(void) const;

};

class StackProxy: public Proxy<StackProxy> {
public:
    StackProxy(ThreadViewBase* m_pThreadViewBase, int index);

    int getIndex(void) const;
    ThreadViewBase* getThread(void) const;
private:
    ThreadViewBase* m_pThreadViewBase;
    int m_absIndex;

};


template<typename CRTP>
inline int Proxy<CRTP>::push(void) const {
    return static_cast<CRTP*>(this)->push(getThread());
}

template<typename CRTP>
template<typename T>
inline Proxy<CRTP>::operator T() const {
    return static_cast<const CRTP*>(this)->template get<T>();
}
/*
template<typename CRTP>
template<typename T>
inline Proxy<CRTP>::operator T&() const {
    return 4;
    //return static_cast<const CRTP*>(this)->get<T&>();
}*/

template<typename CRTP>
inline ThreadViewBase* Proxy<CRTP>::getThread(void) const {
    return static_cast<const CRTP*>(this)->getThread();
}

template<typename CRTP>
inline bool Proxy<CRTP>::isValid(void) const {
    return static_cast<const CRTP*>(this)->isValid();
}


namespace stack_impl {

template<typename P>
struct proxy_stack_pusher {
    static int push(const ThreadViewBase* pBase, StackRecord& record, const P& proxy) {
        return proxy.push(pBase);
    }
};

}

}

#endif // ELYSIAN_LUA_PROXY_HPP
