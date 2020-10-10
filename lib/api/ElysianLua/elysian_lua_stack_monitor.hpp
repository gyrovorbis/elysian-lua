#ifndef ELYSIAN_LUA_STACK_MONITOR_HPP
#define ELYSIAN_LUA_STACK_MONITOR_HPP

#include <cstdint>

#include "elysian_lua_forward_declarations.hpp"
#include "elysian_lua_traits.hpp"
#include "elysian_lua_stack_frame.hpp"
#include "elysian_lua_thread_state.hpp"

namespace elysian::lua {

class ThreadViewBase;

struct MonitorBase{};

template<typename T>
concept Monitorable =
    std::is_base_of_v<MonitorBase, T> &&
    std::is_default_constructible_v<T> &&
    std::is_copy_constructible_v<T> &&
    std::is_assignable_v<T, T> &&
    requires(T t1, typename T::ValueType v) {
        { t1.getCurrentValue() } -> same_as<typename T::ValueType>;
        { v - v }                -> same_as<typename T::ValueType>;
    };

template<typename CRTP, typename T>
class Monitor: MonitorBase {
protected:

    Monitor(T beginVal={}, T endVal={}):
        m_beginValue(std::move(beginVal)),
        m_endValue(std::move(endVal))
    {}

public:
    using ValueType     = T;
    using DerivedType   = CRTP;

    T getBeginValue(void) const;
    T getEndValue(void) const;
    T getCurrentDelta(void) const;
    T getEndDelta(void) const;
    bool isCurrentlyBalanced(void) const;
    bool wasEndBalanced(void) const;

    T begin(void);
    T end(void);

private:

    template<typename D=CRTP>
        requires requires (D d) {
            { d.getCurrentValue() } -> same_as<T>;
        }
    T getCurrentValue(void) const
    {
        return static_cast<const CRTP*>(this)->getCurrentValue();
    }

    T m_beginValue  = {};
    T m_endValue    = {};
};


class StackMonitor:
        public Monitor<StackMonitor, int> {
private:
    const ThreadViewBase* m_pThread;
public:
    StackMonitor(const ThreadViewBase* pThread=nullptr);
    const ThreadViewBase* getThread(void) const;

    int getCurrentValue(void) const;
};

class RegistryRefCountMonitor:
        public Monitor<RegistryRefCountMonitor, int64_t>,
        public StaticThreadStateful
{
public:
    RegistryRefCountMonitor(void);
    int64_t getCurrentValue(void) const;
};


template<Monitorable M, bool RAII=true>
class ScopeGuard: public M  {
public:
    using ValueType = typename M::ValueType;

    ScopeGuard(M monitor={}, ValueType expectedDelta={}, CppExecutionContext ctx=CppExecutionContext());
    ~ScopeGuard(void);

    ValueType getExpectedDelta(void) const;
    const CppExecutionContext& getCppExecutionContext(void) const;
    ValueType end(void);

private:

    CppExecutionContext m_cppCtx;
    ValueType m_expectedDelta = {};
};

template<bool RAII=true>
class StackGuard:
        public ScopeGuard<StackMonitor, RAII>
{
public:
    using ValueType = typename ScopeGuard<StackMonitor, RAII>::ValueType;
    StackGuard(const ThreadViewBase* pThread, ValueType expectedDelta={}, CppExecutionContext ctx = CppExecutionContext());
};

template<bool RAII=true>
class RegistryRefCountGuard:
        public ScopeGuard<RegistryRefCountMonitor, RAII>
{
public:
    using ValueType = typename ScopeGuard<RegistryRefCountMonitor, RAII>::ValueType;
    RegistryRefCountGuard(ValueType expectedDelta={}, CppExecutionContext ctx = CppExecutionContext());
};



template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::getBeginValue(void) const { return m_beginValue; }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::getEndValue(void) const { return m_endValue; }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::getCurrentDelta(void) const { return getCurrentValue() - getBeginValue(); }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::getEndDelta(void) const { return getEndValue() - getBeginValue(); }
template<typename CRTP, typename T>
inline bool Monitor<CRTP, T>::isCurrentlyBalanced(void) const { return getCurrentDelta() == 0; }
template<typename CRTP, typename T>
inline bool Monitor<CRTP, T>::wasEndBalanced(void) const { return getEndDelta() == 0; }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::begin(void) { return m_beginValue = getCurrentValue(); }
template<typename CRTP, typename T>
inline T Monitor<CRTP, T>::end(void) {
    m_endValue = getCurrentValue();
    return getEndDelta();
}

inline const ThreadViewBase* StackMonitor::getThread(void) const { return m_pThread; }

inline RegistryRefCountMonitor::RegistryRefCountMonitor(void):
    Monitor<RegistryRefCountMonitor, int64_t>(getCurrentValue())
{}

template<Monitorable M, bool RAII>
inline ScopeGuard<M, RAII>::ScopeGuard(M monitor, ValueType expectedDelta, CppExecutionContext ctx):
    M(std::move(monitor)),
    m_expectedDelta(std::move(expectedDelta)),
    m_cppCtx(std::move(ctx))
{
    if constexpr(RAII) this->begin();
}

template<Monitorable M, bool RAII>
inline auto ScopeGuard<M, RAII>::getExpectedDelta(void) const -> ValueType { return this->m_expectedDelta; }

template<Monitorable M, bool RAII>
inline ScopeGuard<M, RAII>::~ScopeGuard(void) {
    if constexpr(RAII) end();
}

template<Monitorable M, bool RAII>
inline const CppExecutionContext& ScopeGuard<M, RAII>::getCppExecutionContext(void) const { return this->m_cppCtx; }

template<Monitorable M, bool RAII>
inline auto ScopeGuard<M, RAII>::end(void) -> ValueType {
    const auto delta = M::end();

    [[unlikely]] if(delta != getExpectedDelta()) {
        this->getThread()->setCurrentCppExecutionContext(m_cppCtx);
#if ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_LUA_ERROR
        this->getThread()->error("INVALID STACK DELTA [Expected: %d, Actual: %d, Begin: %d, End: %d]", getExpectedDelta(), delta, this->getBeginValue(), this->getEndValue());
#elif ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_LUA_WARNING
        this->getThread()->warn("INVALID STACK DELTA [Expected: %d, Actual: %d, Begin: %d, End: %d]", getExpectedDelta(), delta, this->getBeginValue(), this->getEndValue());
#elif ELYSIAN_LUA_STACK_GUARD_OPTION == ELYSIAN_LUA_STACK_GUARD_ASSERT
    assert(false);
#endif
        this->getThread()->syncCppCallerContexts();
    }
    return delta;
}

template<bool RAII>
inline StackGuard<RAII>::StackGuard(const ThreadViewBase* pThread, ValueType expectedDelta, CppExecutionContext ctx):
    ScopeGuard<StackMonitor, RAII>(StackMonitor(pThread), std::move(expectedDelta), std::move(ctx)){}

template<bool RAII>
inline RegistryRefCountGuard<RAII>::RegistryRefCountGuard(ValueType expectedDelta, CppExecutionContext ctx):
    ScopeGuard<RegistryRefCountMonitor, RAII>(RegistryRefCountMonitor(), std::move(expectedDelta), std::move(ctx)){}

}

#endif // ELYSIAN_LUA_STACK_MONITOR_HPP
