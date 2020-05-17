#ifndef ELYSIAN_TEST_OPERATORS_HPP
#define ELYSIAN_TEST_OPERATORS_HPP

#include "test_base.hpp"
#include <ElysianLua/elysian_lua_operator_proxy.hpp>

namespace elysian::lua::test {

class OperatorProxyTestSet: public TestSetBase {
    Q_OBJECT
private slots:
    void tableAddOperator(void);

};

inline void OperatorProxyTestSet::tableAddOperator(void) {
    std::array<float, 3> array1 = {
        12.3f,
        -24.45f,
        0.034f
    };
    std::array<bool, 7> array2 = {
        false, true, false, false, false, true, false
    };

    Table table1 = thread().createTable(array1);
    Table table2 = thread().createTable(array2);
    Table meta = thread().createTable(LuaTableValues {
            LuaPair { "__add", static_cast<lua_CFunction>([](lua_State* state) {
                            ThreadView* pThread = Thread::fromState(state);
                          const int length = pThread->length(-2);
                          if(pThread->getType(-1) == LUA_TNUMBER) {
                              pThread->push(length + pThread->toValue<int>(-1));
                          } else if(pThread->getType(-1) == LUA_TTABLE) {
                              pThread->push(length + pThread->length(-1));
                          } else {
                              pThread->push(-117);
                          }
                          return 1;
                          })}
            }
    );

    QVERIFY(table1.setMetaTable(meta));

    auto intAdd = table1 + 3;
    auto tableAdd = table1 + table2;

    QVERIFY(intAdd.isValid());
    QVERIFY(tableAdd.isValid());

    int intVal1 = intAdd;
    QVERIFY(intVal1 == 6);

    int intVal2 = tableAdd;
    QVERIFY(intVal2 == 10);

    thread().push(tableAdd);
    QVERIFY(thread().toValue<int>(-1) == 10);
}

}

#endif // ELYSIAN_TEST_OPERATORS_HPP
