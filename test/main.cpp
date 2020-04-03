#if 0

#include <ElysianLua/elysian_lua_vm.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>
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
ThreadView(lua_State* state);

lua_State *getState(void) const;

// NEWFANGLED-ASS STACK API
int getTop(void) const;
int toAbsStackIndex(int index) const;
int toRelativeStackIndex(int index) const;

operator lua_State *(void)const;

// void push(const LuaVariant &variant);
void push(const char *string); // gracefully push nil for nullptr
void push(const char *pFmt, ...);
void push(const char *pFmt, va_list vaList);
void push(const std::nullptr_t null);
void push(void); // doesn't do shit
void push(lua_Integer integer);
void push(lua_Number number);
void push(const char number);
void push(const bool boolean);
void push(void *lightUd);
#endif

class ThreadViewTest: public QObject {
    Q_OBJECT
private slots:
    void initialize(void);

    void uninitialize(void);
private:

    ThreadView* m_pThreadView = nullptr;

};

void ThreadViewTest::initialize(void) {
    Q_VERIFY(LuaVM::initialize());
}

void ThreadViewTest::uninitialize(void) {
    Q_VERIFY(LuaVM::uninitialize());
}


#if 0
int main(int argc, char *argv[]) {
  LuaVM::initialize();
  printf("HELLO WORLD!!!");
  qDebug() << "Fuck off";
  LuaVM::uninitialize();
  fflush(stdout);
}
#else
    QTEST_MAIN(ThreadViewTest)
#include "threadviewtest.moc"
#endif

#endif
