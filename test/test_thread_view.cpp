#include <ElysianLua/elysian_lua_vm.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>
#include <ElysianLua/elysian_lua_reference.hpp>
#include <ElysianLua/elysian_lua_object.hpp>
#include <QCoreApplication>
#include <cstdio>
#include <cstdlib>
#include <QDebug>
#include <QtTest/QtTest>

using namespace elysian::lua;

/* SHIT TO TEST
 * 1 - assign util functions
    a. atpanic
    b. warn
    c. file I/O
    d. allocator
  2 - Basic Stack manipulation
* 3 - arbitrary code execution
*/

#if 0
// void push(const LuaVariant &variant);
void push(void); // doesn't do shit
void push(std::nullptr_t);
void push(lua_Integer integer);
void push(lua_Number number);
void push(bool boolean);
const char* push(const char* pString); // gracefully push nil for nullptr? Or empty string? wtf?
#ifdef ELYSIAN_LUA_USE_STD_STRING
const char* push(std::string cppStr);
#endif
void push(void* pLightUd); // gracefully push nil for nullptr
void push(const lua_CFunction pCFunc); //gracefully push nil for nullptr

template<typename T, size_t S>
void push(const std::array<T, S>& array);

const char* pushStringFormatted(const char *pFmt, ...);
const char* pushStringVaList(const char *pFmt, va_list vaList);
//const char* pushStringLiteral(const char* pLiteral); MAYBE MACRO
const char* pushStringBuffer(const char* pBuff, size_t length);
void pushGlobalTable(void);
void pushValue(int index);
void pushNil(void);
bool pushThread(void);
void pushCClosure(const lua_CFunction fn, int upvalueCount);

void pop(int count=1);
#endif


class TestThreadView: public QObject {
    Q_OBJECT
private slots:

    void initTestCase(void);
    void pushEmpty(void);
    void pushNil(void);
    void pushNumber(void);
    void pushInteger(void);
    void pushBoolean(void);
    void pushLightUserdata(void);
    void pushString(void);
    void pushStringBuffer(void);
    void pushStringVaList(void);
    void pushStringFormatted(void);
    void pushThread(void);
    void pushCFunction(void);
    void pushCClosure(void);
    void pushValue(void);
    void pushGlobalTable(void);
    void pushArray(void);

    void pushTableValues(void);

    void pop(void);

    void cleanupTestCase(void);

    void cleanup(void);
private:

    ThreadView* m_pThreadView = nullptr;


};

void TestThreadView::initTestCase(void) {
    QVERIFY(LuaVM::initialize());
    m_pThreadView = new ThreadView(*LuaVM::getMainThread());
    QVERIFY(m_pThreadView->isValid());
    QVERIFY(m_pThreadView->getState() == LuaVM::getMainThread()->getState());
    QVERIFY(*m_pThreadView == m_pThreadView->getState());
    QVERIFY(m_pThreadView->getTop() == 0); // starting off with a clean stack
}

void TestThreadView::cleanup(void) {
    lua_pop(*m_pThreadView, m_pThreadView->getTop());
}

void TestThreadView::pushEmpty(void) {
    m_pThreadView->push();
    QVERIFY(m_pThreadView->getTop() == 0);
}

void TestThreadView::pushNil(void) {
    m_pThreadView->push(nullptr);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TNIL);
}

void TestThreadView::pushNumber(void) {
    const float value = 0.1345f;
    m_pThreadView->push(value);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TNUMBER);
    QVERIFY(lua_tonumber(*m_pThreadView, -1) == value);
}

void TestThreadView::pushInteger(void) {
    const int value = -137;
    m_pThreadView->push(value);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TNUMBER);
    QVERIFY(m_pThreadView->isInteger(-1));
    QVERIFY(lua_tointeger(*m_pThreadView, -1) == value);
}

void TestThreadView::pushBoolean(void) {
    const bool value = true;
    m_pThreadView->push(value);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TBOOLEAN);
    QVERIFY(static_cast<bool>(lua_toboolean(*m_pThreadView, -1)) == value);
}

void TestThreadView::pushLightUserdata(void) {
    void* pUdValid = this;
    void* pUdInvalid = nullptr;
    m_pThreadView->push(pUdValid);
    m_pThreadView->push(pUdInvalid);
    QVERIFY(m_pThreadView->getTop() == 2);
    QVERIFY(m_pThreadView->getType(-2) == LUA_TLIGHTUSERDATA);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TNIL);
    QVERIFY(lua_topointer(*m_pThreadView, -2) == pUdValid);
}

void TestThreadView::pushString(void) {
    const char* pStrValid = "HELLO WORLD";
    const char* pStrInvalid = nullptr;
    const char* pStrValidResult = m_pThreadView->push(pStrValid);
    const char* pStrInvalidResult = m_pThreadView->push(pStrInvalid);
    QVERIFY(m_pThreadView->getTop() == 2);
    QVERIFY(m_pThreadView->getType(-2) == LUA_TSTRING);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TNIL);
    QVERIFY(strcmp(pStrValid, pStrValidResult) == 0);
    QVERIFY(!pStrInvalidResult);
    QVERIFY(strcmp(lua_tostring(*m_pThreadView, -2), pStrValid) == 0);
}

void TestThreadView::pushStringBuffer(void) {
    char strBuff[] = "123456789abcdef";
    int offset = 3;
    size_t length = 6;
    char subStrBuff[] = "456789";
    const char* pStrResult = m_pThreadView->pushStringBuffer(strBuff + offset, length);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TSTRING);
    QVERIFY(strcmp(pStrResult, subStrBuff) == 0);
    QVERIFY(strcmp(subStrBuff, lua_tostring(*m_pThreadView, -1)) == 0);
}

void TestThreadView::pushStringVaList(void) {

}

void TestThreadView::pushStringFormatted(void) {
    int intVal = 3;
    int negIntVal = -127;
    char strVal[] = "LUL";
    char format[] = "%d %d %s";
    char buffer[256];
    snprintf(buffer, 256, format, intVal, negIntVal, strVal);
    const char* pStrResult = m_pThreadView->pushStringFormatted(format, intVal, negIntVal, strVal);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TSTRING);
    qDebug() << buffer << pStrResult;
    QVERIFY(strcmp(pStrResult, buffer) == 0);
    QVERIFY(strcmp(buffer, lua_tostring(*m_pThreadView, -1)) == 0);
}

void TestThreadView::pushThread(void) {
    QVERIFY(m_pThreadView->pushThread());
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TTHREAD);
    QVERIFY(lua_tothread(*m_pThreadView, -1) == m_pThreadView->getState());
}

void TestThreadView::pushCFunction(void) {
    lua_CFunction cFunc = [](lua_State*) -> int {
        return 22;
    };
    lua_CFunction bullshitFunc = nullptr;
    m_pThreadView->push(bullshitFunc);
    m_pThreadView->push(cFunc);
    QVERIFY(m_pThreadView->getTop() == 2);
    QVERIFY(m_pThreadView->getType(-2) == LUA_TNIL);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TFUNCTION);
    lua_CFunction cFuncRet = lua_tocfunction(*m_pThreadView, -1);
    QVERIFY(cFuncRet == cFunc);
    QVERIFY(cFuncRet(*m_pThreadView) == 22);
}

void TestThreadView::pushCClosure(void) {
    std::tuple tableValues = {
        std::pair { 1, "a" },
        std::pair { 2, 17 },
        std::pair{ false, 34.0f },
        std::pair{ "inner", std::tuple {
                       std::pair {"a", 2},
                       std::pair {3, "b"}
                   }}
    };
    m_pThreadView->setGlobalsTable("bitchy", tableValues);

    elysian::lua::GlobalsTable globals = m_pThreadView->getGlobalsTable();
    globals["plebe"] = 66;
    int value = globals["plebe"];
    QVERIFY(value == 66);
    globals["bitchy"]["inner"]["a"] = 46;
    auto result = globals["bitchy"]["inner"]["a"];
    value = result;
    QVERIFY(value == 46);
}

void TestThreadView::pushValue(void) {
    const float val = -24.234f;
    m_pThreadView->push(val);
    m_pThreadView->pushValue(-1);
    QVERIFY(m_pThreadView->getTop() == 2);
    QVERIFY(lua_compare(*m_pThreadView, -1, -2, LUA_OPEQ));
}

void TestThreadView::pushGlobalTable(void) {
    m_pThreadView->pushGlobalsTable();
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TTABLE);
    QVERIFY(lua_getfield(*m_pThreadView, -1, "print") == LUA_TFUNCTION);
}

void TestThreadView::pushArray(void) {
    std::array<int, 7> intArray = { 1, 2, 3, 4, 5, 6, 7 };
    m_pThreadView->push(intArray);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TTABLE);
    lua_len(*m_pThreadView, -1);
    QVERIFY(m_pThreadView->isInteger(-1));
    const int length = lua_tointeger(*m_pThreadView, -1);
    QVERIFY(length == 7);
    m_pThreadView->pop();
    for(int i = 1; i <= length; ++i) {
        lua_geti(*m_pThreadView, -1, i);
        QVERIFY(m_pThreadView->isInteger(-1));
        QVERIFY(lua_tointeger(*m_pThreadView, -1) == intArray[i-1]);
        m_pThreadView->pop();
    }
}

void TestThreadView::pushTableValues(void) {
    std::tuple tableValues = {
        std::pair { 1, "a" },
        std::pair{ false, 34.0f },
        std::pair{ "inner", std::tuple {
                       std::pair {"a", 2},
                       std::pair {3, "b"}
                   }}
    };

    m_pThreadView->push(tableValues);

    QVERIFY(m_pThreadView->getType(-1) == LUA_TTABLE);



    m_pThreadView->rPrint(-1);
}

void TestThreadView::pop(void) {
    m_pThreadView->pushGlobalsTable();
    m_pThreadView->pushValue(-1);
    m_pThreadView->push(true);
    m_pThreadView->push("LULUL");
    m_pThreadView->pop(4);
    Q_ASSERT(m_pThreadView->getTop() == 0);
}


void TestThreadView::cleanupTestCase(void) {
    QVERIFY(LuaVM::uninitialize());
}


QTEST_MAIN(TestThreadView)
#include "test_thread_view.moc"

