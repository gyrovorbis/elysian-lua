#ifndef ELYSIAN_TEST_STACK_HPP
#define ELYSIAN_TEST_STACK_HPP

#include <ElysianLua/elysian_lua_vm.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>
#include <ElysianLua/elysian_lua_reference.hpp>
#include <ElysianLua/elysian_lua_object.hpp>
#include <ElysianLua/elysian_lua_table_proxy.hpp>
#include <QCoreApplication>
#include <cstdio>
#include <cstdlib>
#include <QDebug>
#include <QtTest/QtTest>
#include "test_base.hpp"

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
namespace elysian::lua::test {

class StackTestSet: public TestSetBase {
    Q_OBJECT
private slots:

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
    void subscriptOperator(void);

    void tuplePushPull(void);
    void stdOptionalPushPull(void);
    void stdMapPushPull(void);
};

inline void StackTestSet::pushEmpty(void) {
    m_pThreadView->push();
    QVERIFY(m_pThreadView->getTop() == 0);
}

inline void StackTestSet::pushNil(void) {
    m_pThreadView->push(nullptr);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TNIL);
}

inline void StackTestSet::pushNumber(void) {
    const float value = 0.1345f;
    m_pThreadView->push(value);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TNUMBER);
    QVERIFY(lua_tonumber(*m_pThreadView, -1) == value);
}

inline void StackTestSet::pushInteger(void) {
    const int value = -137;
    m_pThreadView->push(value);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TNUMBER);
    QVERIFY(m_pThreadView->isInteger(-1));
    QVERIFY(lua_tointeger(*m_pThreadView, -1) == value);
}

inline void StackTestSet::pushBoolean(void) {
    const bool value = true;
    m_pThreadView->push(value);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TBOOLEAN);
    QVERIFY(static_cast<bool>(lua_toboolean(*m_pThreadView, -1)) == value);
}

inline void StackTestSet::pushLightUserdata(void) {
    void* pUdValid = this;
    void* pUdInvalid = nullptr;
    m_pThreadView->push(pUdValid);
    m_pThreadView->push(pUdInvalid);
    QVERIFY(m_pThreadView->getTop() == 2);
    QVERIFY(m_pThreadView->getType(-2) == LUA_TLIGHTUSERDATA);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TNIL);
    QVERIFY(lua_topointer(*m_pThreadView, -2) == pUdValid);
}

inline void StackTestSet::pushString(void) {
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

inline void StackTestSet::pushStringBuffer(void) {
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

inline void StackTestSet::pushStringVaList(void) {

}

inline void StackTestSet::pushStringFormatted(void) {
    int intVal = 3;
    int negIntVal = -127;
    char strVal[] = "LUL";
    char format[] = "%d %d %s";
    char buffer[256];
    snprintf(buffer, 256, format, intVal, negIntVal, strVal);
    const char* pStrResult = m_pThreadView->pushStringFormatted(format, intVal, negIntVal, strVal);
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TSTRING);
    //qDebug() << buffer << pStrResult;
    QVERIFY(strcmp(pStrResult, buffer) == 0);
    QVERIFY(strcmp(buffer, lua_tostring(*m_pThreadView, -1)) == 0);
}

inline void StackTestSet::pushThread(void) {
    QVERIFY(m_pThreadView->pushThread());
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TTHREAD);
    QVERIFY(lua_tothread(*m_pThreadView, -1) == m_pThreadView->getState());
}

inline void StackTestSet::pushCFunction(void) {
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

inline void StackTestSet::pushCClosure(void) {
#if 0
    LuaTableValues tableValues = {
        std::pair { 1, "a" },
        std::pair { 2, 17 },
        std::pair{ false, 34.0f },
        std::pair{ "inner", LuaTableValues {
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
#endif
}

inline void StackTestSet::pushValue(void) {
    const float val = -24.234f;
    m_pThreadView->push(val);
    m_pThreadView->pushValue(-1);
    QVERIFY(m_pThreadView->getTop() == 2);
    QVERIFY(lua_compare(*m_pThreadView, -1, -2, LUA_OPEQ));
}

inline void StackTestSet::pushGlobalTable(void) {
    m_pThreadView->pushGlobalsTable();
    QVERIFY(m_pThreadView->getTop() == 1);
    QVERIFY(m_pThreadView->getType(-1) == LUA_TTABLE);
    QVERIFY(lua_getfield(*m_pThreadView, -1, "print") == LUA_TFUNCTION);
}

inline void StackTestSet::pushArray(void) {
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

inline void StackTestSet::pushTableValues(void) {
    LuaTableValues tableValues = {
        LuaPair { 1, "a" },
        LuaPair { 32, 34.0f },
        LuaPair { "inner", LuaTableValues {
                       LuaPair {"a", 2},
                       LuaPair { 3, "b"},
                       LuaPair { 4.2f, LuaTableValues {
                                    LuaPair{false, "false"},
                                    LuaPair{1, "true"}
                               }}
                 }}
    };

    m_pThreadView->push(tableValues);

    QVERIFY(m_pThreadView->getType(-1) == LUA_TTABLE);



    m_pThreadView->rPrint(-1);
}

inline void StackTestSet::pop(void) {
    m_pThreadView->pushGlobalsTable();
    m_pThreadView->pushValue(-1);
    m_pThreadView->push(true);
    m_pThreadView->push("LULUL");
    m_pThreadView->pop(4);
    Q_ASSERT(m_pThreadView->getTop() == 0);
}

inline void StackTestSet::subscriptOperator(void) {
    m_pThreadView->setGlobalsTable("bitcher", 3);
    int value = (*m_pThreadView)["bitcher"];
    QVERIFY(value == 3);

    (*m_pThreadView)[347] = "Test";

    const char* pBuffer = (*m_pThreadView)[347];
    QVERIFY(strcmp(pBuffer, "Test") == 0);

}

inline void StackTestSet::tuplePushPull(void) {
    std::tuple pushTable = {
        1,
        23.3f,
        "lolz",
        false,
        reinterpret_cast<void*>(this),
        &StackTestSet::proxyCFunction,
        m_pThreadView->getState()
    };

    decltype(pushTable) pullTable;

    QVERIFY(thread().push(pushTable) == 7);
    QVERIFY(thread().getTop() == 7);

    QVERIFY(thread().pull(pullTable));

    QVERIFY(std::get<0>(pushTable) == std::get<0>(pullTable));
    QVERIFY(std::get<1>(pushTable) == std::get<1>(pullTable));
    QVERIFY(QString(std::get<2>(pushTable)) == std::get<2>(pullTable));
    QVERIFY(std::get<3>(pushTable) == std::get<3>(pullTable));
    QVERIFY(std::get<4>(pushTable) == std::get<4>(pullTable));
    QVERIFY(std::get<5>(pushTable) == std::get<5>(pullTable));
    QVERIFY(std::get<6>(pushTable) == std::get<6>(pullTable));

    QVERIFY(thread().getTop() == 0);
}


inline void StackTestSet::stdOptionalPushPull(void) {
    auto tuple = std::tuple(std::optional<int>(3),
                            std::optional<int>(),
                            std::optional<float>(12.12f),
                            std::optional<float>(),
                            std::optional<const char*>("trOLZOR"),
                            std::optional<const char*>(),
                            std::optional<bool>(true),
                            std::optional<bool>(),
                            std::optional<void*>(reinterpret_cast<void*>(this)),
                            std::optional<void*>(),
                            std::optional<lua_CFunction>(&StackTestSet::proxyCFunction),
                            std::optional<lua_CFunction>(),
                            std::optional<lua_State*>(thread().getState()),
                            std::optional<lua_State*>(),
                            std::optional<Function>(createProxyCFunction()),
                            std::optional<Function>(),
                            std::optional<Table>(thread().getGlobalsTable()),
                            std::optional<Table>());
    decltype(tuple) tuple2;

    QVERIFY(thread().push(tuple));
    QVERIFY(thread().pull(tuple2));
    auto verifyEntry = [&](auto Idx, auto value) {
        QVERIFY(std::get<Idx*2>(tuple2).has_value());
        QVERIFY(std::get<Idx*2>(tuple2).value() == value);
        QVERIFY(!std::get<Idx*2+1>(tuple2).has_value());
    };

    verifyEntry(std::integral_constant<int, 0>(), 3);
    verifyEntry(std::integral_constant<int, 1>(), 12.12f);
    verifyEntry(std::integral_constant<int, 2>(), QString("trOLZOR"));
    verifyEntry(std::integral_constant<int, 3>(), true);
    verifyEntry(std::integral_constant<int, 4>(), reinterpret_cast<void*>(this));
    verifyEntry(std::integral_constant<int, 5>(), &StackTestSet::proxyCFunction);
    verifyEntry(std::integral_constant<int, 6>(), thread().getState());
    verifyEntry(std::integral_constant<int, 7>(), createProxyCFunction());
    verifyEntry(std::integral_constant<int, 8>(), thread().getGlobalsTable());
}

inline void StackTestSet::stdMapPushPull(void) {
    auto intMapSrc = std::map<int, int>{
        { 0, 123 },
        { 1, -17 },
        { 2, 0 },
        { 3, 44444 }
    };
    decltype(intMapSrc) intMapDst;

    auto strMapSrc = std::map<float, int>{
        { -17.8f, 0 },
        { 56664.002f, 1 },
        { 0.0f, 2 },
        { 1.1233f, 3 }
    };
    decltype(strMapSrc) strMapDst;

    QVERIFY(thread().push(intMapSrc));
    QVERIFY(thread().pull(intMapDst));
    QVERIFY(thread().push(strMapSrc));
    QVERIFY(thread().pull(strMapDst));

    QVERIFY(intMapSrc == intMapDst);
    QVERIFY(strMapSrc == strMapDst);
}


}

#endif


