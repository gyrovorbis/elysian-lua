#ifndef ELYSIAN_TEST_BASE_HPP
#define ELYSIAN_TEST_BASE_HPP

#include <ElysianLua/elysian_lua_vm.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>
#include <ElysianLua/elysian_lua_stack_frame.hpp>
#include <ElysianLua/elysian_lua_stack_monitor.hpp>
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
    static int messageHandler(lua_State* pState);
    static void printCppCallerTrace(lua_State* pState);

    Function createProxyCFunction(void) const;

    ThreadView& thread(void) const;

    ThreadView* m_pThreadView = nullptr;
    StackGuard<false>* m_pStackGuard = nullptr;

};

inline ThreadView& TestSetBase::thread(void) const {
    return *m_pThreadView;
}

inline int TestSetBase::atPanic(lua_State* pState) {
    qCritical() << "Lua VM Panic:" << Thread::fromState(pState)->toValue<const char*>(-1);
    printCppCallerTrace(pState);
    assert(false);
    abort();
}

inline int TestSetBase::messageHandler(lua_State *pState) {
    Thread* pThread = Thread::fromState(pState);
    qCritical() << "Protected Call Error:" << pThread->toValue<const char*>(-1);
    printCppCallerTrace(pState);
    return 1;
}

inline void TestSetBase::printCppCallerTrace(lua_State *pState) {
     Thread* pThread = Thread::fromState(pState);
     int stackDepth = pThread->getFunctionCallDepth()-1;
     if(stackDepth <= 0) stackDepth = 1;
     //Walk the call-stack in reverse order checking out the CPP context information.
     for(int i = stackDepth-1; i >= 0; --i) {
         auto pCtx = pThread->getCppCallerContext(i);
         if(pCtx) {
             const CppExecutionContext& ctx = *pCtx;
             if(!ctx.isEmpty()) {
                 //Print out C++ context information!
                 qCritical() << QString("[%4] %2:%3 %1")
                                .arg(QString(ctx.getFileName()).split("/").last()) //not the whole fucking file path.
                                .arg(ctx.getFunctionName())
                                .arg(ctx.getLineNumber()).arg(i);
                 continue;
             }
         }
         qCritical() << QString("[%1]: ?").arg(i);
     }
}

inline void TestSetBase::initTestCase(void) {
    QVERIFY(LuaVM::initialize());
    m_pThreadView = new ThreadView(*LuaVM::getMainThread());
    m_pThreadView->atPanic(&TestSetBase::atPanic);
    m_pThreadView->push(&TestSetBase::messageHandler);
    Function func;
    m_pThreadView->pull(func);
    LuaVM::setGlobalMessageHandler(std::move(func));

    QVERIFY(m_pThreadView->isValid());
    QVERIFY(m_pThreadView->getState() == LuaVM::getMainThread()->getState());
    QVERIFY(*m_pThreadView == m_pThreadView->getState());
    QVERIFY(m_pThreadView->getTop() == 0); // starting off with a clean stack
    m_pStackGuard = new StackGuard<false>(m_pThreadView);
    m_pStackGuard->begin();
}

inline void TestSetBase::cleanup(void) {
    lua_pop(*m_pThreadView, m_pThreadView->getTop());
}

inline void TestSetBase::cleanupTestCase(void) {
    //m_pStackGuard->end(); uncomment me eventually when unit tests are stack-safe
    delete m_pStackGuard;
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
