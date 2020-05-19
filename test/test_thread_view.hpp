#ifndef ELYSIAN_TEST_THREAD_VIEW_HPP
#define ELYSIAN_TEST_THREAD_VIEW_HPP

#include "test_base.hpp"

namespace elysian::lua::test {

class ThreadViewTestSet: public TestSetBase {
    Q_OBJECT
private slots:

    void scriptString(void);
    void scriptFile(void);

};

inline void ThreadViewTestSet::scriptString(void) {
    auto result = thread().script("return 3");
    QVERIFY(result.succeeded());
    QVERIFY(result.getErrorCode() == LUA_OK);
    QVERIFY(result.getReturnCount() == 1);
    int retVal = result;
    QVERIFY(retVal == 3);

    QVERIFY(thread().script("return false") == false);
    QVERIFY(QString(static_cast<const char*>(
                        thread().script("return \"lol\"")))
                    == "lol");
    QVERIFY(thread().script("return 32") == 32);
    QVERIFY(thread().script("return -117") == -117);
    QVERIFY(thread().script("return 1.2") == 1.2f);

    Table envTable = thread().createTable();
    auto floaterResults = thread().script("function floater(a, b) return a + b - 33.333 end", envTable);
    QVERIFY(floaterResults.succeeded());
    QVERIFY(!floaterResults.getReturnCount());
    Function floater = envTable["floater"];
    QVERIFY(floater);
    QVERIFY(floater(1.0035f, -3435.444f) == 1.0035f + -3435.444f - 33.333f);

    auto fResult = thread().script("return 23.3");
    QVERIFY(fResult.succeeded());
    float fretVal = fResult;
    QVERIFY(fretVal == 23.3f);

    auto result2 = thread().script("##$934");
    QVERIFY(!result2.succeeded());
    QVERIFY(!QString(result2.getErrorMessage()).isEmpty());
    QVERIFY(result2.getErrorCode() == LUA_ERRSYNTAX);

    auto result3 = thread().script("randomGlobal = 345");
    QVERIFY(result3.succeeded());
    QVERIFY(!result3.getReturnCount());
    int result3Int = 0;
    QVERIFY(thread().getGlobalsTable("randomGlobal", result3Int) == LUA_TNUMBER);
    QVERIFY(result3Int == 345);

    Table table = thread().script("return {"
                                      "a = 1,"
                                      "[1] = -17,"
                                      "[3] = true,"
                                      "q = 44.41,"
                                      "inner = {"
                                      "   z = _G.print"
                                      "}"
                                  "}");
    QVERIFY(table);
    QVERIFY(table["a"] == 1);
    QVERIFY(table[3] == true);
    QVERIFY(table[1] == -17);
    QVERIFY(table["q"] == 44.41f);
    Function printFunc;
    QVERIFY(thread().getGlobalsTable("print", printFunc) == LUA_TFUNCTION);
    QVERIFY(table["inner"]["z"] == printFunc);

    auto funcResult = thread().script("return function(val1, val2) return val1 + val2 end");
    QVERIFY(funcResult);
    Function func = funcResult;
    QVERIFY(func);
    int funcRetInt = func(3, 4);
    QVERIFY(funcRetInt == 7);

    auto adderFuncResult = thread().script(R"(return function(...)
                       local total = 0
                       for i, v in ipairs{...} do
                           total = total + v
                       end
                       return total
                    end)");
    QVERIFY(adderFuncResult.succeeded());
    Function adderFunc = adderFuncResult;
    QVERIFY(adderFunc);
    int adderRetVal = adderFunc(1, 2, 3, 4, 5);
    QVERIFY(adderRetVal == 15);
#if 0
    QVERIFY(thread().script(R"(return function(...)
                           local total = 1
                           for i, v in ipairs{...} do
                               total = total * v
                           end
                           return total
                        end)")(1, 2, 3, 4) == 24);
#endif
}

inline void ThreadViewTestSet::scriptFile(void)
{
    const char luaScript[] = R"(
        print = 4  -- testng to make sure this doesn't override the default

        function argsToTable(...)
            return {...}
        end

        return "lulz")";

        QTemporaryFile* pTempFile = new QTemporaryFile;
        QVERIFY(pTempFile->open());
        QTextStream out(pTempFile);
        out << luaScript;
        out.flush();
        pTempFile->close();

        Table envTable = thread().createTable();
        auto result = thread().scriptFile(pTempFile->fileName().toStdString().c_str(), envTable);
        if(!result.succeeded()) {
            qDebug() << result.getErrorMessage();
        }
        QVERIFY(result.succeeded());
        QVERIFY(result.getReturnCount() == 1);
        QVERIFY(QString(result.get<const char*>()) == "lulz");

        QVERIFY(envTable["print"] != thread().getGlobalsTable()["print"]);
        QVERIFY(envTable["print"] == 4);
        Function proxyFunc = createProxyCFunction();
        Function argsToTable = envTable["argsToTable"];
        QVERIFY(argsToTable);
        Table varArgsTable = argsToTable(23,
                                         "a",
                                         true,
                                         -33.3f,
                                         reinterpret_cast<void*>(this),
                                         thread().getState(),
                                         &proxyCFunction,
                                         proxyFunc,
                                         thread().getGlobalsTable());
        QVERIFY(varArgsTable);
        QVERIFY(varArgsTable.getLength() == 9);
        QVERIFY(varArgsTable[1] == 23);
        QVERIFY(QString(static_cast<const char*>(varArgsTable[2])) == "a");
        QVERIFY(varArgsTable[3] == true);
        QVERIFY(varArgsTable[4] == -33.3f);
        QVERIFY(varArgsTable[5] == reinterpret_cast<void*>(this));
        QVERIFY(varArgsTable[6] == thread().getState());
        QVERIFY(varArgsTable[7] == &proxyCFunction);
        QVERIFY(varArgsTable[8] == proxyFunc);
        QVERIFY(varArgsTable[9] == thread().getGlobalsTable());

        delete pTempFile;
}

}

#endif // TEST_THREAD_VIEW_HPP
