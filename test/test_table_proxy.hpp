#ifndef ELYSIAN_TEST_TABLE_PROXY_HPP
#define ELYSIAN_TEST_TABLE_PROXY_HPP

#include "test_base.hpp"

namespace elysian::lua::test {

class TableProxyTestSet: public TestSetBase {
    Q_OBJECT
private slots:
    void tableProxyCreate(void);
    void tableProxyPush(void);
    void tableProxyConversion(void);
    void tableProxyAssignment(void);
    void tableProxyComparison(void);
    void tableReferenceCreate(void);

private:

    template<typename P, typename T, typename... Args>
    void genericTableProxyCreate(P&& proxy, const T& table, Args&&... keys);
    template<typename P, typename V>
    void genericTableProxyPush(P&& proxy, V&& expected);
    template<typename P, typename V>
    void genericTableProxyConversion(P&& proxy, const V& expected);

    GlobalsTable proxyGlobals(void) const;
    Table proxyTable(void) const;

    inline const static auto tableValues = LuaTableValues {
            LuaPair { 1, true },
            LuaPair { "2", 22 },
            LuaPair { 4, 12 },
            LuaPair { 3, LuaTableValues {
                LuaPair { 34.0f, reinterpret_cast<void*>(0xdeadbeef) },
                LuaPair { "44", "lol" },
                LuaPair { "5", LuaTableValues {
                    LuaPair { true, LuaTableValues {
                        LuaPair {reinterpret_cast<void*>(0xdeadbeef), 77 },
                        LuaPair { "a", -773.f }
                    }}
                }}
            }}
    };

};



template<typename P, typename T, typename... Args>
inline void TableProxyTestSet::genericTableProxyCreate(P&& proxy, const T& table, Args&&... keys) {
    QVERIFY(proxy.getTable() == table);
    QVERIFY(proxy.getKey() == std::tuple<Args...>(keys...));
}

inline void TableProxyTestSet::tableProxyCreate(void) {
    auto proxy = m_pThreadView->getGlobalsTable()["print"];
    Table table = m_pThreadView->createTable(LuaTableValues {
                                                 LuaPair {"key", "value"}
                                             });
    auto proxy2 = table["key"];
    QVERIFY(proxy.getTable() == m_pThreadView->getGlobalsTable());
    QVERIFY(QString(std::get<0>(proxy.getKey())) == "print");
    QVERIFY(proxy2.getTable() == table);
    QVERIFY(QString(std::get<0>(proxy2.getKey())) == "key");
}

template<typename P, typename V>
inline void TableProxyTestSet::genericTableProxyPush(P&& proxy, V&& expected) {
    V value {};
    m_pThreadView->push(proxy);
    QVERIFY(m_pThreadView->pull(value));
    if constexpr(std::is_same_v<V, const char*>) {
        QVERIFY(strcmp(value, expected) == 0);
    } else {
        QVERIFY(expected == value);
    }
}

inline GlobalsTable TableProxyTestSet::proxyGlobals(void) const {
    m_pThreadView->setGlobalsTable("TableProxyTestSet", tableValues);
    return m_pThreadView->getGlobalsTable();
}

inline Table TableProxyTestSet::proxyTable(void) const {
    return m_pThreadView->createTable(LuaTableValues {
                                          LuaPair { "TableProxyTestSet", tableValues }
                                      });
}

inline void TableProxyTestSet::tableProxyPush(void) {
#define TEST_PROXY(p, v) \
    genericTableProxyPush(proxyGlobals() p, v); \
    genericTableProxyPush(proxyTable() p, v)

    TEST_PROXY(["TableProxyTestSet"][1], true);
    TEST_PROXY(["TableProxyTestSet"]["2"], 22);
    TEST_PROXY(["TableProxyTestSet"][3][34.0f], reinterpret_cast<void*>(0xdeadbeef));
    TEST_PROXY(["TableProxyTestSet"][3]["44"], static_cast<const char*>("lol"));
    TEST_PROXY(["TableProxyTestSet"][3]["5"][true][reinterpret_cast<void*>(0xdeadbeef)], 77);

#undef TEST_PROXY
}

template<typename P, typename V>
inline void TableProxyTestSet::genericTableProxyConversion(P&& proxy, const V& expected) {
    V value = proxy;
    if constexpr(std::is_same_v<V, const char*>) {
        QVERIFY(strcmp(value, expected) == 0);
    } else {
        QVERIFY(expected == value);
    }
}

inline void TableProxyTestSet::tableProxyConversion(void) {
#define TEST_PROXY(p, v) \
    genericTableProxyConversion(proxyGlobals() p, v); \
    genericTableProxyConversion(proxyTable() p, v)

    TEST_PROXY(["TableProxyTestSet"][1], true);
    TEST_PROXY(["TableProxyTestSet"]["2"], 22);
    TEST_PROXY(["TableProxyTestSet"][3][34.0f], reinterpret_cast<void*>(0xdeadbeef));
    TEST_PROXY(["TableProxyTestSet"][3]["44"], static_cast<const char*>("lol"));
    TEST_PROXY(["TableProxyTestSet"][3]["5"][true][reinterpret_cast<void*>(0xdeadbeef)], 77);

#undef TEST_PROXY
}

inline void TableProxyTestSet::tableProxyAssignment(void) {
    Table table = proxyTable();
    GlobalsTable globals = proxyGlobals();
    Table table2 = m_pThreadView->createTable(LuaTableValues{
                                                  LuaPair{"one", "two"}
                                              });
    Table table3;
    Table table4;
    Table table5 = table4;
    QVERIFY(table5 == table4);

#define TEST_PROXY(p, v) \
    globals p = v; \
    genericTableProxyConversion(globals p, v); \
    table p = v; \
    genericTableProxyConversion(table p, v)


    TEST_PROXY(["TableProxyTestSet"][1], 12);
    TEST_PROXY(["TableProxyTestSet"]["2"], false);
    TEST_PROXY(["TableProxyTestSet"][3][34.0f], nullptr);
    TEST_PROXY(["TableProxyTestSet"][3]["44"], static_cast<const char*>("lolz"));
    TEST_PROXY(["TableProxyTestSet"][3]["5"][true][reinterpret_cast<void*>(0xdeadbeef)], table2);
    table3 = table["TableProxyTestSet"][3]["5"][true][reinterpret_cast<void*>(0xdeadbeef)];

    QVERIFY(table3 == table2);
    QVERIFY(table3 == table["TableProxyTestSet"][3]["5"][true][reinterpret_cast<void*>(0xdeadbeef)]);

#undef TEST_PROXY
}

inline void TableProxyTestSet::tableProxyComparison(void) {
    Table table = proxyTable();
    GlobalsTable globals = proxyGlobals();

    QVERIFY(table["TableProxyTestSet"][3]["5"][true][reinterpret_cast<void*>(0xdeadbeef)] ==
            globals["TableProxyTestSet"][3]["5"][true][reinterpret_cast<void*>(0xdeadbeef)]);

    QVERIFY(globals["TableProxyTestSet"][3]["44"] !=
            table["TableProxyTestSet"][3]["5"][true][reinterpret_cast<void*>(0xdeadbeef)]);
}

inline void TableProxyTestSet::tableReferenceCreate(void) {
    Table table = proxyTable();

    TableFieldRef<Table, int> intField = &table["TableProxyTestSet"][4];
    TableFieldRef<Table, const char*> strField = &table["TableProxyTestSet"][3]["5"][true]["a"];

    QVERIFY(intField == 12);
    QVERIFY(strField == -773.f);

    intField = 34;
    strField = static_cast<void*>(this);

    QVERIFY(intField == 34);
    QVERIFY(strField == static_cast<void*>(this));
}

}

#endif
