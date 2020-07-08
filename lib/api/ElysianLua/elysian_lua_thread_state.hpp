#ifndef ELYSIAN_LUA_THREAD_STATE_HPP
#define ELYSIAN_LUA_THREAD_STATE_HPP

#include <concepts>

namespace elysian::lua {

class ThreadViewBase;

template<typename T>
concept ThreadStateful =
    requires(T t) {
        { t.getThread() } -> std::same_as<const ThreadViewBase*>;
    };

template<typename T>
concept ModifiableThreadStateful =
    ThreadStateful<T> &&
    requires(T t, const ThreadViewBase* pThreadView) {
        { t.setThread(pThreadView) };
    };

template<typename CRTP>
class ThreadStatefulBase {
public:
    using ThreadStatefulBaseType = CRTP;
};

class StaticThreadStateful {
public:
    static const ThreadViewBase* staticThread(void);
    const ThreadViewBase* getThread(void) const;
};

class ExplicitThreadStateful {
public:
    const ThreadViewBase* getThread(void) const { return m_pThread; }
    bool setThread(const ThreadViewBase* pThreadView) { m_pThread = pThreadView; return true; }

private:
    const ThreadViewBase* m_pThread = nullptr;

};

}



#endif // ELYSIAN_LUA_THREAD_STATE_HPP
