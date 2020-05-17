#ifndef ELYSIAN_TEST_BASE_HPP
#define ELYSIAN_TEST_BASE_HPP

#include <ElysianLua/elysian_lua_vm.hpp>
#include <ElysianLua/elysian_lua_thread_view.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>
#include "elysian_qtest.hpp"

namespace elysian::lua::test {

class TestSetBase: public elysian::UnitTestSuite {
    Q_OBJECT
protected slots:

    void initTestCase(void);
    void cleanupTestCase(void);
    void cleanup(void);

protected:

    static int proxyCFunction(lua_State* pState);
    static int atPanic(lua_State* pState);

    Function createProxyCFunction(void) const;

    ThreadView& thread(void) const;

    ThreadView* m_pThreadView = nullptr;

};

inline ThreadView& TestSetBase::thread(void) const {
    return *m_pThreadView;
}

inline int TestSetBase::atPanic(lua_State* pState) {
    qCritical() << "Lua VM Panic:" << Thread::fromState(pState)->toValue<const char*>(-1);
    assert(false);
    abort();
}

inline void TestSetBase::initTestCase(void) {
    QVERIFY(LuaVM::initialize());
    m_pThreadView = new ThreadView(*LuaVM::getMainThread());
    m_pThreadView->atPanic(&TestSetBase::atPanic);
    QVERIFY(m_pThreadView->isValid());
    QVERIFY(m_pThreadView->getState() == LuaVM::getMainThread()->getState());
    QVERIFY(*m_pThreadView == m_pThreadView->getState());
    QVERIFY(m_pThreadView->getTop() == 0); // starting off with a clean stack
}

inline void TestSetBase::cleanup(void) {
    lua_pop(*m_pThreadView, m_pThreadView->getTop());
}

inline void TestSetBase::cleanupTestCase(void) {
    QVERIFY(LuaVM::uninitialize());
}

inline int TestSetBase::proxyCFunction(lua_State* pState) {
    Thread* pThread = Thread::fromState(pState);
    const int args = pThread->getTop();
    for(int i = 1; i <= args; ++i) {
        pThread->pushValue(i);
    }
    return args;
}

inline Function TestSetBase::createProxyCFunction(void) const {
    Function func;

    [&]() {
        QVERIFY(thread().push(&TestSetBase::proxyCFunction));
        QVERIFY(thread().pull(func));
    }();

    return func;
}

}


#endif // TEST_BASE_HPP
