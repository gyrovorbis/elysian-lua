#ifndef ELYSIAN_LUA_PROXY_HPP
#define ELYSIAN_LUA_PROXY_HPP

#include "elysian_lua_reference.hpp"

namespace elysian::lua {

class ThreadViewBase;

template<typename CRTP>
class Proxy: public StatelessRefBase<CRTP> {
public:

    //provides
    template<typename T>
#if 0

    requires requires(const CRTP b) {
        { b.template get<T>() } -> std::same_as<T>;
    }
#endif

    operator T() const {
        return static_cast<const CRTP*>(this)->template get<T>();
    }

    // Explicitly override the boolean conversion operator we inherited from StatelessRefBase to
    // abide by Proxy conversion semantics.
    explicit operator bool(void) const { return static_cast<const CRTP*>(this)->template get<bool>(); }

    template<typename T = CRTP>
    requires requires (T b, const ThreadViewBase* pThread) {
        { b.push(pThread) } -> same_as<bool>;
    }
    bool push(const ThreadViewBase* pThread) const
    {
        return static_cast<const CRTP*>(this)->push(pThread);
    }

    int makeStackIndex(const ThreadViewBase* pThread) const {
        assert(pThread);
        int index = 0;
        if(push(pThread)) {
            index = pThread->toAbsStackIndex(-1);
        }
        return index;
    }

    int makeStackIndex(void) const {
        return makeStackIndex(static_cast<const CRTP*>(this)->getThread());
    }

    bool doneWithStackIndex(int index) const {
        return doneWithStackIndex(static_cast<const CRTP*>(this)->getThread(), index);
    }

    bool doneWithStackIndex(const ThreadViewBase* pThread, int index) const {
        if(index) {
            pThread->remove(index);
            return true;
        } else {
            return false;
        }
    }

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

/*
template<typename CRTP>
template<typename T>
inline Proxy<CRTP>::operator T&() const {
    return 4;
    //return static_cast<const CRTP*>(this)->get<T&>();
}*/



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



