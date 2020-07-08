#ifndef ELYSIAN_TEST_TABLE_HPP
#define ELYSIAN_TEST_TABLE_HPP

#include <ElysianLua/elysian_lua_thread.hpp>
#include <ElysianLua/elysian_lua_operator_proxy.hpp>
#include <ElysianLua/elysian_lua_reference.hpp>
#include <ElysianLua/elysian_lua_table.hpp>
#include <ElysianLua/elysian_lua_forward_declarations.hpp>

#include <QTemporaryFile>
#include <QDebug>

#include "test_base.hpp"
namespace elysian::lua::test {


template<typename TableType>
class TableTestSetBase: public TestSetBase {
protected:

    void tableCreateFromStack(void);
    void tableCreateNewEmpty(void);
    void tablePush(void);
    void tableCreateCopyConstruct(void);
    void tableCreateNewTableValues(void);
    void tableCreateArray(void);
    void tableCompare(void);
    void tableGetField(void);
    void tableSetField(void);
    void tableIterate(void);
    void tableAppendValues(void);
    void tableAppendTable(void);
    void tableMetaTable(void);

    //C++11-style iteration
    //Metatables

public:

    template<typename T>
    TableType fromTable(T&& table);
};

class TableTestSet:
        public TableTestSetBase<Table>
{
    using ParentTestSet = TableTestSetBase<Table>;
    Q_OBJECT
private slots:
    void tableCreateDefault(void);
    void tableCreateFromGlobals(void);


    ELYSIAN_INHERIT_TEST_CASE(tableCreateFromStack)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateNewEmpty)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateCopyConstruct)
    ELYSIAN_INHERIT_TEST_CASE(tablePush)
    void tablePull(void);
    ELYSIAN_INHERIT_TEST_CASE(tableCreateNewTableValues)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateArray)
    ELYSIAN_INHERIT_TEST_CASE(tableCompare)
    ELYSIAN_INHERIT_TEST_CASE(tableGetField)
    ELYSIAN_INHERIT_TEST_CASE(tableSetField)
    ELYSIAN_INHERIT_TEST_CASE(tableIterate)
    ELYSIAN_INHERIT_TEST_CASE(tableAppendValues)
    ELYSIAN_INHERIT_TEST_CASE(tableAppendTable)
    ELYSIAN_INHERIT_TEST_CASE(tableMetaTable)
};

class StackTableTestSet:
    public TableTestSetBase<StackTable>
{
    using ParentTestSet = TableTestSetBase<StackTable>;
    Q_OBJECT
private slots:

    ELYSIAN_INHERIT_TEST_CASE(tableCreateFromStack)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateNewEmpty)
    ELYSIAN_INHERIT_TEST_CASE(tablePush)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateNewTableValues)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateArray)
    ELYSIAN_INHERIT_TEST_CASE(tableCompare)
    ELYSIAN_INHERIT_TEST_CASE(tableGetField)
    ELYSIAN_INHERIT_TEST_CASE(tableSetField)
    ELYSIAN_INHERIT_TEST_CASE(tableIterate)
    ELYSIAN_INHERIT_TEST_CASE(tableAppendValues)
    ELYSIAN_INHERIT_TEST_CASE(tableAppendTable)
    ELYSIAN_INHERIT_TEST_CASE(tableMetaTable)
};

class StaticTableTestSet:
        public TableTestSetBase<StaticTable>
{
    using ParentTestSet = TableTestSetBase<StaticTable>;
    Q_OBJECT
private slots:
    //void tableCreateDefault(void);
    //void tableCreateFromGlobals(void);

    ELYSIAN_INHERIT_TEST_CASE(tableCreateFromStack)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateNewEmpty)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateCopyConstruct)
    ELYSIAN_INHERIT_TEST_CASE(tablePush)
    //void tablePull(void);
    ELYSIAN_INHERIT_TEST_CASE(tableCreateNewTableValues)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateArray)
    ELYSIAN_INHERIT_TEST_CASE(tableCompare)
    ELYSIAN_INHERIT_TEST_CASE(tableGetField)
    ELYSIAN_INHERIT_TEST_CASE(tableSetField)
    ELYSIAN_INHERIT_TEST_CASE(tableIterate)
    ELYSIAN_INHERIT_TEST_CASE(tableAppendValues)
    ELYSIAN_INHERIT_TEST_CASE(tableAppendTable)
    ELYSIAN_INHERIT_TEST_CASE(tableMetaTable)
};

class StaticStackTableTestSet:
        public TableTestSetBase<StaticStackTable>
{
    using ParentTestSet = TableTestSetBase<StaticStackTable>;
    Q_OBJECT
private slots:

    ELYSIAN_INHERIT_TEST_CASE(tableCreateFromStack)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateNewEmpty)
    ELYSIAN_INHERIT_TEST_CASE(tablePush)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateNewTableValues)
    ELYSIAN_INHERIT_TEST_CASE(tableCreateArray)
    ELYSIAN_INHERIT_TEST_CASE(tableCompare)
    ELYSIAN_INHERIT_TEST_CASE(tableGetField)
    ELYSIAN_INHERIT_TEST_CASE(tableSetField)
    ELYSIAN_INHERIT_TEST_CASE(tableIterate)
    ELYSIAN_INHERIT_TEST_CASE(tableAppendValues)
    ELYSIAN_INHERIT_TEST_CASE(tableAppendTable)
    ELYSIAN_INHERIT_TEST_CASE(tableMetaTable)
};

inline void TableTestSet::tableCreateDefault(void) {
    Table table;
    QVERIFY(!table);
    QVERIFY(!table.isValid());
}

inline void TableTestSet::tableCreateFromGlobals(void) {
    Table table = m_pThreadView->getGlobalsTable();
    QVERIFY(table);
    QVERIFY(table.isValid());
}

inline void TableTestSet::tablePull(void) {
    Table nilTable;
    Table realTable;
    m_pThreadView->pushNil();
    m_pThreadView->pull(nilTable);
    QVERIFY(nilTable.getType() == LUA_TNONE);
    m_pThreadView->pushNewTable();
    m_pThreadView->pull(realTable);
    QVERIFY(realTable.getType() == LUA_TTABLE);
    QVERIFY(!nilTable);
    QVERIFY(realTable);
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableCreateFromStack(void) {
    m_pThreadView->pushNil();
    TableType table = m_pThreadView->toValue<TableType>(-1);
    QVERIFY(!table);

    m_pThreadView->pushNewTable();
    TableType table2 = m_pThreadView->toValue<TableType>(-1);
    QVERIFY(table2);

    m_pThreadView->pushGlobalsTable();
    TableType table3 = m_pThreadView->toValue<TableType>(-1);
    QVERIFY(table3);
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableCreateNewEmpty(void) {
    TableType table = m_pThreadView->createTable();
    QVERIFY(table);
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableCreateCopyConstruct(void) {
    TableType table1 = thread().createTable();
    TableType table2 = table1;
    QVERIFY(thread().push(table1));
    QVERIFY(thread().push(table2));
    QVERIFY(thread().compare(-1, -2, LUA_OPEQ));
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tablePush(void) {
    thread().pushNewTable();
    TableType table = thread().toValue<TableType>(-1);
    QVERIFY(m_pThreadView->push(table));
    QVERIFY(m_pThreadView->getType(-1) == LUA_TTABLE);
    QVERIFY(m_pThreadView->compare(-1, -2, LUA_OPEQ));
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableCreateNewTableValues(void) {
    TableType table = m_pThreadView->createTable(LuaTableValues{
        LuaPair{ 1, -1 },
        LuaPair{ 2, "b" },
        LuaPair{ 3, true },
        LuaPair{ 4, static_cast<void*>(this) }

    });
    int intVal = 0;
    const char* strVal = "";
    bool boolVal = false;
    void* lightUdVal = nullptr;
    QVERIFY(table);
    QVERIFY(m_pThreadView->push(table));
    QVERIFY(m_pThreadView->getType(-1) == LUA_TTABLE);
    QVERIFY(table.getLength() == 4);

    QVERIFY(m_pThreadView->getTable(-1, 1, intVal) == LUA_TNUMBER);
    QVERIFY(intVal == -1);
    QVERIFY(m_pThreadView->getTableRaw(-1, 2, strVal) == LUA_TSTRING);
    QVERIFY(strcmp(strVal, "b") == 0);
    QVERIFY(m_pThreadView->getTable(-1, 3, boolVal) == LUA_TBOOLEAN);
    QVERIFY(boolVal == true);
    QVERIFY(m_pThreadView->getTableRaw(-1, 4, lightUdVal) == LUA_TLIGHTUSERDATA);
    QVERIFY(lightUdVal == this);
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableCreateArray(void) {
    const char* array[3] = {
        "test",
        "lol",
        "wtf"
    };
    std::array<float, 3> array2 = {
        12.3f,
        -24.45f,
        0.034f
    };
    const char* sVal = "";
    float fVal = 0.0f;

    TableType table = m_pThreadView->createTable(array);

    QVERIFY(table);
    QVERIFY(table.getLength() == 3);
    QVERIFY(m_pThreadView->push(table));

    QVERIFY(m_pThreadView->getTable(-1, 1, sVal) == LUA_TSTRING);
    QVERIFY(strcmp(sVal, "test") == 0);
    QVERIFY(m_pThreadView->getTableRaw(-1, 2, sVal) == LUA_TSTRING);
    QVERIFY(strcmp(sVal, "lol") == 0);
    QVERIFY(m_pThreadView->getTable(-1, 3, sVal) == LUA_TSTRING);
    QVERIFY(strcmp(sVal, "wtf") == 0);

    TableType table2 = m_pThreadView->createTable(array2);
    QVERIFY(table2);
    QVERIFY(table2.getLength() == 3);
    QVERIFY(table2.getLengthRaw() == 3);
    QVERIFY(m_pThreadView->push(table2));
    QVERIFY(m_pThreadView->getTableRaw(-1, 1, fVal) == LUA_TNUMBER);
    QVERIFY(fVal == 12.3f);
    QVERIFY(m_pThreadView->getTable(-1, 2, fVal) == LUA_TNUMBER);
    QVERIFY(fVal == -24.45f);
    QVERIFY(m_pThreadView->getTableRaw(-1, 3, fVal) == LUA_TNUMBER);
    QVERIFY(fVal == 0.034f);
}


template<typename TableType>
template<typename T>
inline TableType TableTestSetBase<TableType>::fromTable(T&& table) {
    if constexpr(StackReferenceable<TableType>) {
        thread().push(std::forward<T>(table));
        return thread().toValue<TableType>(-1);
    } else {
        return std::forward<T>(table);
    }
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableCompare(void) {
    TableType nullTable;
    GlobalsTable globals = m_pThreadView->getGlobalsTable();
    TableType table1 = m_pThreadView->createTable();
    TableType table2 = fromTable(globals);
    TableType table3 = table1;
    TableType table4 = table2;
    QVERIFY(nullTable != globals);
    QVERIFY(nullTable != table1);
    QVERIFY(nullTable != table2);
    QVERIFY(nullTable != table4);
    QVERIFY(globals != table1);
    QVERIFY(globals == table2);
    QVERIFY(globals != table3);
    QVERIFY(globals == table4);
    QVERIFY(table1 != table2);
    QVERIFY(table1 == table3);
    QVERIFY(table1 != table4);
    QVERIFY(table2 != table3);
    QVERIFY(table2 == table4);
    QVERIFY(table3 != table4);
#if 0
    m_pThreadView->pushNewTable();
    TableType table5 = m_pThreadView->toValue<TableType>(-1);
    TableType table6 = m_pThreadView->toValue<TableType>(-1);
    QVERIFY(table5 == table6);
    QVERIFY(table5);
    table5 = nullTable;
    QVERIFY(!table5);
    QVERIFY(table5 == nullTable);
    QVERIFY(table6);
    table6 = nullptr;
    QVERIFY(!table6);
    QVERIFY(table6 == table5);
#endif
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableGetField(void) {
    GlobalsTable globals = m_pThreadView->getGlobalsTable();
    TableType table = fromTable(globals);
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

    QVERIFY(table.getField("testInt", testInt) == LUA_TNUMBER);
    QVERIFY(testInt == 33);
    QVERIFY(table.getFieldRaw("testFloat", testFloat) == LUA_TNUMBER);
    QVERIFY(testFloat == 32.1f);
    QVERIFY(table.getField("testString", testString) == LUA_TSTRING);
    QVERIFY(strcmp(testString, "lulz") == 0);
    QVERIFY(table.getFieldRaw("testBool", testBool) == LUA_TBOOLEAN);
    QVERIFY(testBool == true);
    QVERIFY(table.getField("testLightUd", testLightUd) == LUA_TLIGHTUSERDATA);
    QVERIFY(testLightUd == this);
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableSetField(void) {
    GlobalsTable globals = m_pThreadView->getGlobalsTable();
    TableType table = fromTable(globals);
    table.setField("test2Int", 33);
    table.setFieldRaw("test2Float", 32.1f);
    table.setField("test2String", "lulz");
    table.setFieldRaw("test2Bool", true);
    table.setField("test2LightUd", static_cast<void*>(this));

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

template<typename TableType>
inline void TableTestSetBase<TableType>::tableIterate(void) {
    int array[] = {
        2, -14, -46, 23, 111, 2345, -23
    };
    int iterations = 0;

    TableType table = m_pThreadView->createTable(array);
    table.iterate([&]() {
        int key = m_pThreadView->toValue<int>(-2) - 1; // 1-based Lua => 0-based C
        int value = m_pThreadView->toValue<int>(-1);
        QVERIFY(array[key] == value);
        ++iterations;
    });
    QVERIFY(iterations == sizeof(array)/sizeof(array[0]));
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableAppendValues(void) {
    TableType table = m_pThreadView->createTable();

    table += LuaTableValues {
        LuaPair { "test", true },
        LuaPair { 2, 45.3f }
    };

    bool bVal = false;
    float fVal = 0.0f;

    QVERIFY(table.getField("test", bVal) == LUA_TBOOLEAN);
    QVERIFY(bVal == true);
    QVERIFY(table.getFieldRaw(2, fVal) == LUA_TNUMBER);
    QVERIFY(fVal == 45.3f);
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableAppendTable(void) {
    TableType table1 = m_pThreadView->createTable(std::array<int, 3>{
        1, 2, 3
    });

    TableType table2 = m_pThreadView->createTable(LuaTableValues {
      LuaPair { 4, 4 },
      LuaPair { 5, 5 }
   });

    int iterations = 0;
    int intVal = 0;
    table1 += table2;

    table1.iterate([&]() {
        int key = m_pThreadView->toValue<int>(-2);
        int value = m_pThreadView->toValue<int>(-1);
        QVERIFY(key == value);
        ++iterations;
    });
    QVERIFY(iterations == 5);
    QVERIFY(table1.getFieldRaw(4, intVal) == LUA_TNUMBER);
    QVERIFY(intVal == 4);
    QVERIFY(table1.getField(5, intVal) == LUA_TNUMBER);
    QVERIFY(intVal == 5);
}

template<typename TableType>
inline void TableTestSetBase<TableType>::tableMetaTable(void) {

    TableType table = thread().createTable();
    TableType meta = thread().createTable(LuaTableValues {
            LuaPair { "__index", LuaTableValues{
                LuaPair { "a", 45.3f },
                LuaPair { 3, true }
            }
        }});

    QVERIFY(table.setMetaTable(meta));
    Table meta2 = table.template getMetaTable<Table>();

    QVERIFY(table["a"] == 45.3f);
    QVERIFY(table[3] == true);
    QVERIFY(meta == meta2);
}


}

#endif
