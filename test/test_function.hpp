#ifndef ELYSIAN_TEST_FUNCTION_HPP
#define ELYSIAN_TEST_FUNCTION_HPP

#include "test_base.hpp"
#include <ElysianLua/elysian_lua_function_result.hpp>
#include <ElysianLua/elysian_lua_table_proxy.hpp>
#include <ElysianLua/elysian_lua_stack_frame.hpp>
#include <ElysianLua/elysian_lua_variant.hpp>
#include <ElysianLua/elysian_lua_protected_block.hpp>

namespace elysian::lua::test {

class FunctionTestSet: public TestSetBase {
    Q_OBJECT
private slots:

    void functionCreate(void);
    void functionPush(void);
    void functionComparison(void);
    void functionAssignment(void);
    void functionCall(void);
    void functionReturnValues(void);
    void functionReturnTuples(void);
    void functionReturnSubTuples(void);
    void functionReturnStructuredBindings(void);
    void functionReturnStdTie(void);
    void functionReturnMove(void);
    void functionReturnPush(void);
    void functionChaining(void);

    void protectedScope(void);
    void contextualCall(void);
    void stackGuard(void);

private:

    template<typename... Args>
    void functionProxyArgRetVals(const Function& function, Args&&... args) const;
    template<typename... Args>
    void functionProxyArgRetTuple(const Function& function, Args&&... args) const;
    template<typename R, typename V, std::size_t... Is>
    void functionProxyCheckRetVal(const R& results, const V& valueTuple, std::index_sequence<Is...>) const;
    template<typename R, typename V, std::size_t... Is>
    void functionProxyCheckRetTuple(const R& resultTuple, const V& valueTuple, std::index_sequence<Is...>) const;
};


inline void FunctionTestSet::functionCreate(void) {
    Function function1;
    Function function2 = thread()["print"];
    Function function3 = thread()["string"]["format"];
    QVERIFY(!function1.isValid());
    QVERIFY(!function1);
    QVERIFY(function2.isValid());
    QVERIFY(function2);
    QVERIFY(function3);
    QVERIFY(function3.isValid());
}

inline void FunctionTestSet::functionPush(void) {
    Function function1;
    Function function2 = thread()["print"];
    Function function3 = thread()["string"]["format"];

    QVERIFY(thread().push(function1));
    thread().pushNil();
    QVERIFY(thread().compare(-1, -2, LUA_OPEQ));
    QVERIFY(thread().push(function2));
    QVERIFY(thread().getGlobalsTable("print"));
    QVERIFY(thread().compare(-1, -2, LUA_OPEQ));
    QVERIFY(thread().push(function3));
    QVERIFY(thread().push(thread()["string"]["format"]));
    QVERIFY(thread().compare(-1, -2, LUA_OPEQ));
}

inline void FunctionTestSet::functionComparison(void) {
    Function function1;
    Function function2 = thread()["print"];
    Function function3 = thread()["string"]["format"];
    QVERIFY(function1 == nullptr);
    QVERIFY(function1 != function2);
    QVERIFY(function1 != function3);
    QVERIFY(function2 != function3);
    QVERIFY(function2 == thread()["print"]);
    QVERIFY(function3 == thread()["string"]["format"]);
}

inline void FunctionTestSet::functionAssignment(void) {
    Function function1;
    Function function3 = function1;
    Function function2 = thread()["print"];
    Function function4 = function2;
    Function function5;

    QVERIFY(function3 == function1);
    QVERIFY(function3 != function2);
    QVERIFY(function3 != function4);
    QVERIFY(function4 == function2);
    QVERIFY(function3 == nullptr);
    QVERIFY(function4 != nullptr);
    function5 = std::move(function4);
    QVERIFY(function5 == function2);
    QVERIFY(function5 != function4);
    QVERIFY(!function4);
    QVERIFY(function4 == function3);
}

inline void FunctionTestSet::functionCall(void) {

    lua_CFunction testCFunc = [](lua_State* pState) -> int {
        Thread* pThread = Thread::fromState(pState);
        pThread->setTable(-3);
        return 0;
    };
    Function func;
    Table table = thread().createTable();
    Table table2 = thread().createTable(LuaTableValues {
                                            LuaPair { "lol", "table" }
                                        });

#define CALL_VERIFY_FUNC(KEY, VALUE) \
    { \
        QVERIFY(thread().push(testCFunc)); \
        QVERIFY(thread().pull(func)); \
        QVERIFY(func); \
        auto result = func(table, KEY, VALUE); \
        if(!result.succeeded()) \
            qDebug() << result.getErrorMessage(); \
        else QVERIFY(table[KEY] == VALUE); \
    }

    CALL_VERIFY_FUNC(1, 33.0f);
    CALL_VERIFY_FUNC("lol", "test");
    CALL_VERIFY_FUNC(17.33f, false);
    CALL_VERIFY_FUNC(false, thread().getState());
    CALL_VERIFY_FUNC(5, reinterpret_cast<void*>(this));
    CALL_VERIFY_FUNC(6, table2);
    CALL_VERIFY_FUNC(table2, 7);
    CALL_VERIFY_FUNC(7, func);

#undef CALL_VERIFY_FUNC
}

template<typename... Args>
inline void FunctionTestSet::functionProxyArgRetVals(const Function& function, Args&&... args) const {
    auto retVals = function(args...);
    if(!retVals.succeeded()) {
        qDebug() << retVals.getErrorMessage();
        QVERIFY(false);
    } else {
        functionProxyCheckRetVal(retVals, std::forward_as_tuple(args...), std::index_sequence_for<Args&&...>());
    }
}

template<typename R, typename V, std::size_t... Is>
inline void FunctionTestSet::functionProxyCheckRetVal(const R& results, const V& valueTuple, std::index_sequence<Is...>) const {
    auto confirmValue = [&](auto Idx) {
        auto val1 = results.template get<std::decay_t<decltype(std::get<Idx>(valueTuple))>>(Idx);
        auto val2 = std::get<Idx>(valueTuple);
        if constexpr(std::is_same_v<const char*, std::decay_t<decltype(std::get<Idx>(valueTuple))>>) {
            QVERIFY(QString(val1) == val2);
        } else {
            QVERIFY(val1 == val2);
        }
    };

    (confirmValue(std::integral_constant<std::size_t, Is>()), ...);
}

inline void FunctionTestSet::functionReturnValues(void) {

    Function func = createProxyCFunction();

    functionProxyArgRetVals(func,
                            1,
                            23.3f,
                            "lolz",
                            false,
                            Table(thread().getGlobalsTable()),
                            reinterpret_cast<void*>(this),
                            func,
                            &FunctionTestSet::proxyCFunction,
                            m_pThreadView->getState());
}

template<typename R, typename V, std::size_t... Is>
inline void FunctionTestSet::functionProxyCheckRetTuple(const R& resultTuple, const V& valueTuple, std::index_sequence<Is...>) const {
    auto confirmValue = [&](auto Idx) {
        auto val1 = std::get<Idx>(resultTuple);
        auto val2 = std::get<Idx>(valueTuple);
        if constexpr(std::is_same_v<const char*, std::decay_t<decltype(std::get<Idx>(valueTuple))>>) {
            QVERIFY(QString(val1) == val2);
        } else {
            QVERIFY(val1 == val2);
        }
    };

    (confirmValue(std::integral_constant<std::size_t, Is>()), ...);
}

template<typename... Args>
inline void FunctionTestSet::functionProxyArgRetTuple(const Function& function, Args&&... args) const {
    auto argTuple = std::make_tuple(args...);
    auto retVals = function(argTuple);

    if(!retVals.succeeded()) {
        qDebug() << retVals.getErrorMessage();
        QVERIFY(false);
    } else {
        decltype(argTuple) retTuple = retVals;
        functionProxyCheckRetTuple(retTuple, argTuple, std::index_sequence_for<Args...>());
    }
}

inline void FunctionTestSet::functionReturnTuples(void) {
    Function func = createProxyCFunction();

    functionProxyArgRetTuple(func,
         1,
         23.3f,
         "lolz",
         false,
         Table(thread().getGlobalsTable()),
         reinterpret_cast<void*>(this),
         func,
         &FunctionTestSet::proxyCFunction,
         m_pThreadView->getState());
}

inline void FunctionTestSet::functionReturnSubTuples(void) {
    Function func = createProxyCFunction();
    std::tuple argTuple = {
        1,
        23.3f,
        "lolz",
        false,
        Table(thread().getGlobalsTable()),
        reinterpret_cast<void*>(this),
        func,
        &FunctionTestSet::proxyCFunction,
        m_pThreadView->getState()
    };
    auto retVal = func(argTuple);
    QVERIFY(retVal.succeeded());
    QVERIFY(retVal.getReturnCount() == 9);
    QVERIFY(retVal.get<int>(0) == 1);
    QVERIFY(retVal.get<Function>(7) == func);
    QVERIFY(retVal.get<lua_State*>(8) == thread().getState());

    auto val1 = retVal.get<std::tuple<int>>(0);
    auto val2 = retVal.get<std::tuple<float, const char*, bool>>(1);
    auto val3 = retVal.get<std::tuple<lua_State*>>(8);

    QVERIFY(std::get<0>(val1) == 1);
    QVERIFY(std::get<0>(val2) == 23.3f);
    QVERIFY(QString(std::get<1>(val2)) == "lolz");
    QVERIFY(std::get<0>(val3) == m_pThreadView->getState());
}

inline void FunctionTestSet::functionReturnStructuredBindings(void) {
    Function func = createProxyCFunction();
    std::tuple argTuple = {
        1,
        23.3f,
        "lolz",
        false
    };
    auto retVal = func(argTuple);

    int iVal = 0;
    float fVal = 0.0f;
    const char* sVal = nullptr;
    bool bVal = true;

    auto[intVal, floatVal, stringVal, boolVal] = static_cast<decltype(argTuple)>(retVal);

    QVERIFY(intVal == 1);
    QVERIFY(floatVal == 23.3f);
    QVERIFY(QString(stringVal) == "lolz");
    QVERIFY(boolVal == false);
}

inline void FunctionTestSet::functionReturnStdTie(void) {
    Function func = createProxyCFunction();
    std::tuple argTuple = {
        1,
        23.3f,
        "lolz",
        false
    };
    auto retVal = func(argTuple);

    int iVal = 0;
    float fVal = 0.0f;
    const char* sVal = nullptr;
    bool bVal = true;

#if 0
    std::tie(iVal, fVal, sVal, bVal) = retVal;

    QVERIFY(intVal == 1);
    QVERIFY(floatVal == 23.3f);
    QVERIFY(QString(stringVal) == "lolz");
    QVERIFY(boolVal == false);
#endif
}

inline void FunctionTestSet::functionReturnMove(void) {
    Function func = createProxyCFunction();
    std::tuple argTuple = {
        1,
        23.3f,
        "lolz",
        false
    };
    auto retVal = func(argTuple);
    auto retVal2 = std::move(retVal);

    QVERIFY(!retVal.getReturnCount());
    QVERIFY(retVal2.getReturnCount() == 4);

    int iVal = 0;
    float fVal = 0.0f;
    const char* sVal = nullptr;
    bool bVal = true;

    auto[intVal, floatVal, stringVal, boolVal] = static_cast<decltype(argTuple)>(retVal2);

    QVERIFY(intVal == 1);
    QVERIFY(floatVal == 23.3f);
    QVERIFY(QString(stringVal) == "lolz");
    QVERIFY(boolVal == false);
}

inline void FunctionTestSet::functionReturnPush(void) {
    Function func = createProxyCFunction();
    std::tuple argTuple = {
        1,
        23.3f,
        reinterpret_cast<void*>(this),
        false
    };
    decltype(argTuple) retTuple;
    QVERIFY(thread().push(func(argTuple)));
    QVERIFY(thread().pull(retTuple));
    QVERIFY(retTuple == argTuple);
}

inline void FunctionTestSet::functionChaining(void) {
    Function func = createProxyCFunction();

    std::tuple<int,
            std::optional<float>,
            void*,
            std::optional<bool>,
            const char*,
            Table> retTuple = func(func(func(std::make_tuple(
                                                1,
                                                23.3f,
                                                reinterpret_cast<void*>(this),
                                                true
                                            ))), func("fuckinA", thread().getGlobalsTable()));

    QVERIFY(std::get<0>(retTuple) == 1);
    QVERIFY(std::get<1>(retTuple).value() == 23.3f);
    QVERIFY(std::get<2>(retTuple) == reinterpret_cast<void*>(this));
    QVERIFY(std::get<3>(retTuple).value() == true);
    QVERIFY(QString(std::get<4>(retTuple)) == "fuckinA");
    QVERIFY(std::get<5>(retTuple) == thread().getGlobalsTable());

}


inline void FunctionTestSet::protectedScope(void) {
    ELYSIAN_LUA_PROTECTED_BLOCK(&thread()) {

        ELYSIAN_LUA_PROTECTED_BLOCK(&thread()) {

            ELYSIAN_LUA_PROTECTED_BLOCK(&thread()) {

                ELYSIAN_LUA_PROTECTED_BLOCK(&thread()) {
                    int illegalIndex = thread().getTop();
                    thread().error("Fuck off!");
                };

            };

        };

    };
}


inline void FunctionTestSet::contextualCall(void) {
    auto errorFunc = [](lua_State* pState) -> int {
        char buffer[2048] = { 0 };
        Thread* pThread = Thread::fromState(pState);

            const char* str = nullptr;
            for(int i = 1; i <= pThread->getTop(); ++i) {
                str = pThread->toValue<const char*>(i);
                if(str) {
                    strcat(buffer, str);
                    strcat(buffer, " ");
                }
            }
            ELYSIAN_LUA_PROTECTED_BLOCK(pThread) {
                pThread->error("Shit's reested: %s", buffer);
            };
        return 1;
    };

    ELYSIAN_LUA_PROTECTED_BLOCK(&thread()) {

        Function func;
        QVERIFY(thread().push(static_cast<lua_CFunction>(errorFunc)));
        QVERIFY(thread().pull(func));

        ELYSIAN_LUA_CONTEXTUAL_CALL(func, "ReesterBell", "mcReestly", "abadkf", 44, false, -3443.0f);

    };
}

inline void FunctionTestSet::stackGuard(void) {
#if 0
    ELYSIAN_LUA_PROTECTED_BLOCK(&thread()) {
        ELYSIAN_LUA_STACK_GUARD(&thread());
        thread().pushNil();
    };
#endif
}

}



#endif // TEST_FUNCTION_HPP
