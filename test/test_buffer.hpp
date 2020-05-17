#ifndef ELYSIAN_TEST_BUFFER_HPP
#define ELYSIAN_TEST_BUFFER_HPP

#include "test_base.hpp"
#include <ElysianLua/elysian_lua_buffer.hpp>

namespace elysian::lua::test {

class BufferTestSet: public TestSetBase {
    Q_OBJECT
private slots:

    void bufferStatic(void);
    void bufferDynamic(void);
    void bufferStream(void);

};

inline void BufferTestSet::bufferStatic(void) {
    char str[] = "Hello Bitches!";
    Buffer buffer;
    QVERIFY(!buffer.isValid());
    QVERIFY(buffer.isEmpty());
    QVERIFY(thread().bufferInitSize(&buffer, sizeof(str)));
    QVERIFY(buffer.isValid());
    QVERIFY(buffer.getAddress() != nullptr);
    QVERIFY(static_cast<const char*>(buffer) == buffer.getAddress());
    buffer = str;
    buffer += "jesus christ";
    //QVERIFY(buffer.getLength() == strlen(str));
    buffer.pushResult();
    qDebug() << thread().toValue<const char*>(-1);
    //QVERIFY(QString(thread().toValue<const char*>(-1)) == str);
    thread().pop(thread().getTop());
    qDebug() << buffer;
   // QVERIFY(QString(buffer) == str);
}

inline void BufferTestSet::bufferDynamic(void) {
    Buffer buffer;
    thread().bufferInit(&buffer);
    buffer.addGSub("%s: ", "%s", "Replaced");

    Table table = thread().createTable();
    Table meta = thread().createTable(LuaTableValues{
                                          LuaPair { "__name", "MetaShit" }
                                      });
    table.setMetaTable(meta);

    buffer += table;
    buffer += 132144;
    buffer += reinterpret_cast<void*>(this);
    buffer += "fuckwad";

    qDebug() << buffer;

    buffer.pushResult();
    qDebug() << thread().toValue<const char*>(-1);
}

inline void BufferTestSet::bufferStream(void) {
    Buffer buffer;
    thread().bufferInit(&buffer);

    buffer << "hello" << "my" << "good" << 32.0f << thread().getState() << thread().getGlobalsTable();
    qDebug() << buffer;

}

}


#endif // TEST_BUFFER_HPP
