#ifndef ELYSIAN_TEST_GLOBALS_TABLE_HPP
#define ELYSIAN_TEST_GLOBALS_TABLE_HPP

#include "test_base.hpp"

namespace elysian::lua::test {

class GlobalsTableTestSet: public TestSetBase {
    Q_OBJECT

private slots:

    void globalsTableCreate(void);
    void globalsTableGetField(void);
    void globalsTableSetField(void);
    void globalsTablePush(void);
    void globalsTableCompareGlobalsTable(void);
    void globalsTableCopyConstruct(void);
    void globalsTableAssignment(void);

};

inline void GlobalsTableTestSet::globalsTableCreate(void) {
    QVERIFY(sizeof(lua_Integer) == 4);
    if constexpr(!std::is_same_v<lua_Number, float>)
    QVERIFY(false);

    GlobalsTable nullTable;
    GlobalsTable globals = m_pThreadView->getGlobalsTable();
    QVERIFY(globals);
    QVERIFY(globals.isValid());
    QVERIFY(globals.getType() == LUA_TTABLE);
    QVERIFY(!nullTable);
    QVERIFY(!nullTable.isValid());
}

inline void GlobalsTableTestSet::globalsTableGetField(void) {
    GlobalsTable globals = m_pThreadView->getGlobalsTable();
    int testInt = 0;
    float testFloat = 0.0f;
    const char* testString = "";
    bool testBool = false;
    void* testLightUd = nullptr;

    m_pThreadView->setGlobalsTable("testInt", 33);
    m_pThreadView->setGlobalsTable("testFloat", 32.1f);
    m_pThreadView->setGlobalsTable("testString", "lulz");
    m_pThreadView->setGlobalsTable("testBool", true);
    m_pThreadView->setGlobalsTable("testLightUd", static_cast<void*>(this));

    QVERIFY(globals.getField("testInt", testInt) == LUA_TNUMBER);
    QVERIFY(testInt == 33);
    QVERIFY(globals.getField("testFloat", testFloat) == LUA_TNUMBER);
    QVERIFY(testFloat == 32.1f);
    QVERIFY(globals.getField("testString", testString) == LUA_TSTRING);
    QVERIFY(strcmp(testString, "lulz") == 0);
    QVERIFY(globals.getField("testBool", testBool) == LUA_TBOOLEAN);
    QVERIFY(testBool == true);
    QVERIFY(globals.getField("testLightUd", testLightUd) == LUA_TLIGHTUSERDATA);
    QVERIFY(testLightUd == this);
}

inline void GlobalsTableTestSet::globalsTableSetField(void) {
    GlobalsTable globals = m_pThreadView->getGlobalsTable();
    globals.setField("test2Int", 33);
    globals.setField("test2Float", 32.1f);
    globals.setField("test2String", "lulz");
    globals.setField("test2Bool", true);
    globals.setField("test2LightUd", static_cast<void*>(this));

    int testInt = 0;
    float testFloat = 0.0f;
    const char* testString = "";
    bool testBool = false;
    void* testLightUd = nullptr;

    QVERIFY(globals.getField("test2Int", testInt) == LUA_TNUMBER);
    QVERIFY(testInt == 33);
    QVERIFY(globals.getField("test2Float", testFloat) == LUA_TNUMBER);
    QVERIFY(testFloat == 32.1f);
    QVERIFY(globals.getField("test2String", testString) == LUA_TSTRING);
    QVERIFY(strcmp(testString, "lulz") == 0);
    QVERIFY(globals.getField("test2Bool", testBool) == LUA_TBOOLEAN);
    QVERIFY(testBool == true);
    QVERIFY(globals.getField("test2LightUd", testLightUd) == LUA_TLIGHTUSERDATA);
    QVERIFY(testLightUd == this);
}

inline void GlobalsTableTestSet::globalsTablePush(void) {
    GlobalsTable globals = m_pThreadView->getGlobalsTable();
    int intVal = 0;
    globals.setField("pushConfirmation", -17);
    m_pThreadView->push(globals);
    QVERIFY(m_pThreadView->getTable(-1, "pushConfirmation", intVal) == LUA_TNUMBER);
    QVERIFY(intVal == -17);
}

inline void GlobalsTableTestSet::globalsTableCompareGlobalsTable(void) {
    GlobalsTable nullTable;
    Table tablet;
    Table tabletter;
    GlobalsTable globals1 = m_pThreadView->getGlobalsTable();
    GlobalsTable globals2 = m_pThreadView->getGlobalsTable();
    QVERIFY(globals1 == globals2);
    QVERIFY(globals1 != nullTable);
}

inline void GlobalsTableTestSet::globalsTableCopyConstruct(void) {
    GlobalsTable globals1 = m_pThreadView->getGlobalsTable();
    GlobalsTable globals2 = globals1;
    QVERIFY(globals1 == globals2);
}

inline void GlobalsTableTestSet::globalsTableAssignment(void) {
    GlobalsTable globals1;
    GlobalsTable globals2 = m_pThreadView->getGlobalsTable();
    GlobalsTable globals3;
    QVERIFY(globals1 != globals2);
    globals1 = globals2;
    QVERIFY(globals1 == globals2);
    globals3 = globals1;
    QVERIFY(globals3 == globals1);
    globals3 = nullptr;
    QVERIFY(!globals3);
}

}

#endif // ELYSIAN_TEST_GLOBALS_TABLE_HPP
