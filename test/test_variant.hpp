#ifndef TEST_VARIANT_HPP
#define TEST_VARIANT_HPP

#include "test_base.hpp"

namespace elysian::lua::test {

//integer tests

class VariantTestSet: public TestSetBase  {
    Q_OBJECT

    constexpr static const char*const _luaFunc =
                                        "function addShit(a, b)\n"
                                        "     return a+b\n"
                                        "end\n"
                                        "return addShit";

private Q_SLOTS:

    //void createNew(void);
    //void createFromReference(void);
    //constructors
    void createWithNilDefaultType(void);
    void createWithNullPtrType(void);
    void createWithBoolType(void);
    void createWithIntegerType(void);
    void createWithNumberType(void);
    void createWithStringType(void);
    void createWithVoidPtrType(void);
    void createWithUserdataType(void);
    void createFromStdInitializerList(void);
    void createFromPairsStdInitializerList(void);
    void createFromStack(void);
    void createFromArray(void);
    void createNewTable(void);
    //void createFromRef(void);

    //fetching values
    void isNil(void);
    void getBool(void);
    void getInteger(void);
    void getNumber(void);
    void getString(void);
    void getVoidPtr(void);
    void getUserdata(void);

    void compareNullPtr(void);
    void compareVariant(void);
    void compareBool(void);
    void compareInteger(void);
    void compareNumber(void);
    void compareString(void);
    void compareVoidPtr(void);
    void compareUserdata(void);

    void setNil(void);
    void setVariant(void);
    void setBool(void);
    void setInteger(void);
    void setNumber(void);
    void setString(void);
    void setVoidPtr(void);
    void setUserdata(void);

    void assignNullPtr(void);
    void assignBool(void);
    void assignInteger(void);
    void assignNumber(void);
    void assignString(void);
    void assignVoidPtr(void);
    void assignUserdata(void);

    void pushNil(void);
    void pullNil(void);
    void pushBool(void);
    void pullBool(void);
    void pushNumber(void);
    void pullNumber(void);
    void pushInteger(void);
    void pullInteger(void);
    void pushString(void);
    void pullString(void);
    void pushVoidPtr(void);
    void pullVoidPtr(void);
    void pushUserdata(void);
    void pullUserdata(void);
    void pushLuaFunction(void);
    void pullLuaFunction(void);

    //boolean conversions
    void nilAsBool(void);
    void boolAsBool(void);
    void integerAsBool(void);
    void numberAsBool(void);
    void stringAsBool(void);
    void voidPtrAsBool(void);

    //number conversions
    void nilAsNumber(void);
    void boolAsNumber(void);
    void numberAsNumber(void);
    void integerAsNumber(void);
    void stringAsNumber(void);
    void voidPtrAsNumber(void);

    //integer conversions
    void nilAsInteger(void);
    void boolAsInteger(void);
    void numberAsInteger(void);
    void integerAsInteger(void);
    void stringAsInteger(void);
    void voidPtrAsInteger(void);

    //string conversions
    void nilAsString(void);
    void boolAsString(void);
    void numberAsString(void);
    void integerAsString(void);
    void stringAsString(void);
    void voidPtrAsString(void);

    void voidPtrAsVoidPtr(void);
    void otherAsVoidPtr(void);

    //function operations
    void callLuaFunctionIntReturn(void);
    void callLuaFunctionVariadicArgsVoidReturn(void);
    void callCppLambdaClosure(void);

    void makeWeakRef(void);
    void getLength(void);

    //metatables
    //void setMetatable(void);
    //void getMetatable(void);
    //getTypeName() userdata
    //fromString()

    //table iteration
    void iterateTable(void);
    void iterateTableConst(void);
    void iterateTableCConst(void);
    void iterateTableRangedBasedFor(void);
    void iterateTableModifyFields(void);
};

inline void VariantTestSet::createWithNilDefaultType(void) {
    Variant variant;
    QVERIFY(variant.getType() == LUA_TNIL);
}

inline void VariantTestSet::createWithNullPtrType(void) {
    Variant variant(nullptr);
    QVERIFY(variant.getType() == LUA_TNIL);
}

inline void VariantTestSet::createWithBoolType(void) {
    Variant variant(true);
    QVERIFY(variant.getType() == LUA_TBOOLEAN);
}

inline void VariantTestSet::createWithIntegerType(void) {
    Variant variant(31);
    QVERIFY(variant.getType() == LUA_TNUMBER);
    QVERIFY(variant.isInteger());
}

inline void VariantTestSet::createWithNumberType(void) {
    Variant variant(33.0f);
    QVERIFY(variant.getType() == LUA_TNUMBER);
}

inline void VariantTestSet::createWithStringType(void) {
    Variant variant("stringTest");
    QVERIFY(variant.getType() == LUA_TSTRING);
}

inline void VariantTestSet::createWithVoidPtrType(void) {
    int i = 33;
    Variant variant((void*)&i);
    QVERIFY(variant.getType() == LUA_TLIGHTUSERDATA);
}

inline void VariantTestSet::createWithUserdataType(void) {
#if 0
    elysian::InstanceTransform transform;
    Variant variant(&transform);
    QVERIFY(variant.getType() == LUA_TUSERDATA);
#endif
}

inline void VariantTestSet::createFromStdInitializerList(void) {
    Variant variant({{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}});
    QVERIFY(variant.getType() == LUA_TTABLE);
    QVERIFY(variant.getLength() == 5);
}

inline void VariantTestSet::createFromPairsStdInitializerList(void) {
    //elysian::Vector2 testUd;

    Variant v = {
        {"field1", 3},
        {"field2", false},
        {1, "Testzy"},
        {"LOLOLOL", {
            { 0, 33.333f },
            {"tragler", true },
            {"twoLayersDeepTable", {
                //{"randomAssUserdata", &testUd},
               #if 0
                 {"randomAssLambdaClosure", [&](const float val) {
                    // testUd.setX(val);
                     return val;
                 }}
             #endif
             }}
        }},
        {"array", Variant::fromArray({
            1,
            22.0f,
            "fuck",
            false//,
            //LuaManager::exec("return function(s) return '%s: '..s end", "DYNAMIC_ASS_STRING")
        })},

        { 1, "Weeeeell!!"},
        { 2, 8 },
        //{ &testUd, "Arbitrary userdata as an index... because I can!"},
        {"So fucking epic", false}//,
        //{22.8, (void*)this}
    };

    QVERIFY(v.getType() == LUA_TTABLE);
    //QVERIFY((*v["LOLOLOL"]["twoLayersDeepTable"]["randomAssLambdaClosure"])(-17.77f).getValue<float>() == -17.77f);
    //QVERIFY(v["LOLOLOL"]["twoLayersDeepTable"]["randomAssUserdata"]["x"]->getValue<float>() == -17.77f);
    //QVERIFY((*v["array"][5])("randomString") == "DYNAMIC_ASS_STRING: randomString");
}


#if 0 //NEEDS TO BE FUCKING FIXED!!!!
        {"constCharArg", [&](char*string) {
            return string;
        }},
#endif
#if 0
    QVERIFY((*v["constCharArg"])("textz0r") == "textz0r");
#endif

inline void VariantTestSet::createFromStack(void) {
    lua_pushinteger(thread(), 3);
    QVERIFY(Variant::fromStack(-1) == 3);
}

inline void VariantTestSet::createFromArray(void) {
   //elysian::Vector2 vex;

    Variant v = Variant::fromArray({
        1,
        false,
        2.2f,
        "fuckme",
        (void*)this,
        //OOLUA::Lua_table_ref(),
        //OOLUA::Lua_func_ref(),
        //&vex,
#if 0
        [&](const int value) {
            return value;
        },
#endif
        static_cast<lua_CFunction>([](lua_State*) {
            return 0;
        })
    });

    //QVERIFY(v.getLength() == 10);
    QVERIFY(v.getLength() == 6);

}

inline void VariantTestSet::createNewTable(void) {
    Variant variant = Variant::newTable();
    QVERIFY(variant.getType() == LUA_TTABLE);
    QVERIFY(variant.getRef() != LUA_REFNIL);
    QVERIFY(variant.getRef() != LUA_NOREF);
}

inline void VariantTestSet::isNil(void) {
    Variant variant(nullptr);
    QVERIFY(variant.isNil());
}

inline void VariantTestSet::getBool(void) {
    Variant variant(true);
    QVERIFY(variant.getValue<bool>() == true);
}

inline void VariantTestSet::getInteger(void) {
    Variant variant(3);
    QVERIFY(variant.getValue<lua_Integer>() == 3);
}

inline void VariantTestSet::getNumber(void) {
    lua_Number num = 13.0f;
    Variant variant(num);
    QVERIFY(variant.getValue<lua_Number>() == num);
}

inline void VariantTestSet::getString(void) {
    Variant variant("StringTest");
    QVERIFY((strcmp(variant.getValue<const char*>(), "StringTest") == 0));
}

inline void VariantTestSet::getVoidPtr(void) {
    int i = 3;
    Variant variant((void*)&i);
    QVERIFY(variant.getValue<void*>() == &i);
}

inline void VariantTestSet::getUserdata(void) {
#if 0
    elysian::InstanceTransform transform;
    Variant variant(&transform);
    QVERIFY(variant.getValue<elysian::InstanceTransform*>() == &transform);
#endif
}

inline void VariantTestSet::compareNullPtr(void) {
    QVERIFY(Variant(nullptr) == nullptr);
}

inline void VariantTestSet::compareVariant(void) {
    //elysian::Vector2 vec;
    QVERIFY(Variant(nullptr) == Variant(nullptr));
    QVERIFY(Variant(true) == Variant(true));
    QVERIFY(Variant(0) == Variant(0));
    QVERIFY(Variant(-23) == Variant(-23));
    QVERIFY(Variant(0.3f) == Variant(0.3f));
    QVERIFY(Variant("FuckYou") == Variant("FuckYou"));
    //QVERIFY(Variant(&vec) == Variant(&vec));
}

inline void VariantTestSet::compareBool(void) {
    QVERIFY(Variant(true) == true);
}

inline void VariantTestSet::compareInteger(void) {
    QVERIFY(Variant(3) == 3);
}

inline void VariantTestSet::compareNumber(void) {
    lua_Number num = 14.4f;
    QVERIFY(Variant(num) == num);
}

inline void VariantTestSet::compareString(void) {
    QVERIFY(Variant("stringitator") == "stringitator");
}

inline void VariantTestSet::compareVoidPtr(void) {
    int i = 33;
    QVERIFY(Variant((void*)&i) == (void*)&i);
}

inline void VariantTestSet::compareUserdata(void) {
#if 0
    elysian::InstanceTransform transform;
    QVERIFY(Variant(&transform) == &transform);
#endif
}

inline void VariantTestSet::setNil(void) {
    Variant variant1(4);
    Variant variant2(4);
    variant1.setValue(nullptr);
    variant2.setNil();
    QVERIFY(variant1 == Variant());
    QVERIFY(variant2 == Variant());
    QVERIFY(variant1 == variant2);
}

inline void VariantTestSet::setVariant(void) {
    //elysian::Vector2 vec;
    Variant nil;
    Variant boolean(true);
    Variant intNum(3);
    Variant floatNum(3.33f);
    Variant str("fucktwats");
    //Variant userdata(&vec);

    Variant v;
    v = nil;
    QVERIFY(v == nil);
    v = boolean;
    QVERIFY(v == boolean);
    v = intNum;
    QVERIFY(v == intNum);
    v = floatNum;
    QVERIFY(v == floatNum);
    v = str;
    QVERIFY(v == str);
    //v = userdata;
    //QVERIFY(v == userdata);
}

inline void VariantTestSet::setBool(void) {
    Variant nil;
    nil.setValue(true);
    QVERIFY(nil == Variant(true));
    nil.setValue(false);
    QVERIFY(nil == Variant(false));
}

inline void VariantTestSet::setInteger(void) {
    Variant v;
    v.setValue(44);
    QVERIFY(v == Variant(44));
}

inline void VariantTestSet::setNumber(void) {
    Variant v;
    v.setValue(44.2f);
    QVERIFY(v == Variant(44.2f));
}

inline void VariantTestSet::setString(void) {
    Variant v;
    v.setValue("LOLSTR");
    QVERIFY(v == Variant("LOLSTR"));
}

inline void VariantTestSet::setVoidPtr(void) {
    Variant v;
    v.setValue((void*)this);
    QVERIFY(v == Variant((void*)this));
}

inline void VariantTestSet::setUserdata(void) {
#if 0
    elysian::InstanceTransform transform;
    Variant v;
    v.setValue(&transform);
    QVERIFY(v == Variant(&transform));
#endif
}


inline void VariantTestSet::assignNullPtr(void) {
    Variant variant(33.0f);
    variant = nullptr;
    QVERIFY(variant == nullptr);
}

inline void VariantTestSet::assignBool(void) {
    Variant variant;
    variant = true;
    QVERIFY(variant == true);
}

inline void VariantTestSet::assignInteger(void) {
    Variant variant;
    variant = 12;
    QVERIFY(variant == 12);
}

inline void VariantTestSet::assignNumber(void) {
    Variant variant;
    lua_Number num = static_cast<lua_Number>(44.0f);
    variant = num;
    QVERIFY(variant == num);
}

inline void VariantTestSet::assignString(void) {
    Variant variant;
    variant = "Fuck You";
    QVERIFY(variant == "Fuck You");
}

inline void VariantTestSet::assignVoidPtr(void) {
    int i = 3;
    Variant variant;
    variant = (void*)&i;
    QVERIFY(variant == (void*)&i);
}

inline void VariantTestSet::assignUserdata(void) {
#if 0
    elysian::InstanceTransform transform;
    Variant variant;
    variant = &transform;
    QVERIFY(variant == &transform);
#endif
}


inline void VariantTestSet::pushNil(void) {
    Variant variant;
    variant.push();
    const int luaType = lua_type(thread(), -1);
    lua_pop(thread(), 1);
    QVERIFY(luaType == LUA_TNIL);
}

inline void VariantTestSet::pullNil(void) {
    Variant variant;
    lua_pushnil(thread());
    variant.pull();
    QVERIFY(variant.isNil());
}

inline void VariantTestSet::pushBool(void) {
    Variant variant(true);
    variant.push();
    const int luaType = lua_type(thread(), -1);
    const bool val = lua_toboolean(thread(), -1);
    lua_pop(thread(), 1);
    QVERIFY(luaType == LUA_TBOOLEAN);
    QVERIFY(val == true);
}

inline void VariantTestSet::pullBool(void) {
    Variant variant;
    lua_pushboolean(thread(), true);
    variant.pull();
    QVERIFY(variant.getType() == LUA_TBOOLEAN);
    QVERIFY(variant.getValue<bool>() == true);
}

inline void VariantTestSet::pushInteger(void) {
    Variant variant(4);
    variant.push();
    const int luaType = lua_type(thread(), -1);
    QVERIFY(luaType == LUA_TNUMBER);
    QVERIFY(lua_isinteger(thread(), -1));
    const lua_Integer val = lua_tointeger(thread(), -1);
    lua_pop(thread(), 1);
    QVERIFY(val == 4);
}

inline void VariantTestSet::pullInteger(void) {
    Variant variant;
    lua_pushinteger(thread(), 24);
    variant.pull();
    QVERIFY(variant.getType() == LUA_TNUMBER);
    QVERIFY(variant.getValue<lua_Integer>() == 24);
}


inline void VariantTestSet::pushNumber(void) {
    lua_Number num = static_cast<lua_Number>(33.0f);
    Variant variant(num);
    variant.push();
    const int luaType = lua_type(thread(), -1);
    const lua_Number val = lua_tonumber(thread(), -1);
    lua_pop(thread(), 1);
    QVERIFY(luaType == LUA_TNUMBER);
    QVERIFY(val == num);
}

inline void VariantTestSet::pullNumber(void) {
    lua_Number num = static_cast<lua_Number>(12.0f);
    Variant variant;
    lua_pushnumber(thread(), num);
    variant.pull();
    QVERIFY(variant.getType() == LUA_TNUMBER);
    QVERIFY(variant.getValue<float>() == num);
}

inline void VariantTestSet::pushString(void) {
    Variant variant("FUCK YOU!!!");
    variant.push();
    const int luaType = lua_type(thread(), -1);
    const char*const val = lua_tostring(thread(), -1);
    lua_pop(thread(), 1);
    QVERIFY(luaType == LUA_TSTRING);
    QVERIFY(strcmp(val, "FUCK YOU!!!") == 0);
}

inline void VariantTestSet::pullString(void) {
    Variant variant;
    lua_pushstring(thread(), "biiiitches!!!");
    variant.pull();
    QVERIFY(variant.getType() == LUA_TSTRING);
    QVERIFY(strcmp(variant.getValue<const char*>(), "biiiitches!!!") == 0);
}

inline void VariantTestSet::pushVoidPtr(void) {
    int i = 22;
    Variant variant((void*)&i);
    variant.push();
    const int luaType = lua_type(thread(), -1);
    void*const val = lua_touserdata(thread(), -1);
    lua_pop(thread(), 1);
    QVERIFY(luaType == LUA_TLIGHTUSERDATA);
    QVERIFY(val == &i);
}

inline void VariantTestSet::pullVoidPtr(void) {
    int i = 22;
    Variant variant;
    lua_pushlightuserdata(thread(), &i);
    variant.pull();
    QVERIFY(variant.getType() == LUA_TLIGHTUSERDATA);
    QVERIFY(variant.getValue<void*>() == &i);
}

inline void VariantTestSet::pushUserdata(void) {
#if 0
    elysian::InstanceTransform transform;
    Variant variant(&transform);
    variant.push();
    const int luaType = lua_type(thread(), -1);
    elysian::InstanceTransform* ptr = nullptr;
    OOLUA::pull(thread(), ptr);
    QVERIFY(luaType == LUA_TUSERDATA);
    QVERIFY(ptr == &transform);
#endif
}

inline void VariantTestSet::pullUserdata(void) {
#if 0
    elysian::InstanceTransform transform;
    Variant variant;
    OOLUA::push(thread(), &transform);
    variant.pull();
    QVERIFY(variant.getType() == LUA_TUSERDATA);
    QVERIFY(variant.getValue<elysian::InstanceTransform*>() == &transform);
#endif
}

inline void VariantTestSet::pullLuaFunction(void) {
    Variant variant;
    thread().push(thread().getGlobalsTable()["print"]);
    QVERIFY(variant.pull());
    QVERIFY(variant.getType() == LUA_TFUNCTION);
}

inline void VariantTestSet::pushLuaFunction(void) {
    thread().getGlobalsTable("print");
    int ref = luaL_ref(thread(), LUA_REGISTRYINDEX);
    QVERIFY(ref != LUA_REFNIL);
    QVERIFY(ref != LUA_NOREF);
    Variant variant;
    variant.setRef(ref);
    QVERIFY(variant.push());
    QVERIFY(lua_isfunction(thread(), -1));
    lua_rawgeti(thread(), LUA_REGISTRYINDEX, ref);
    QVERIFY(lua_compare(thread(), -1, -2, LUA_OPEQ));
    lua_pop(thread(), 2);
    luaL_unref(thread(), LUA_REGISTRYINDEX, ref);
}


inline void VariantTestSet::nilAsBool(void) {
    Variant v;
    QVERIFY(v.asBool() == false);
}

inline void VariantTestSet::boolAsBool(void) {
    Variant v(false);
    QVERIFY(v.asBool() == false);
    v = true;
    QVERIFY(v.asBool() == true);
}

inline void VariantTestSet::integerAsBool(void) {
    Variant v(0);
    QVERIFY(v.asBool() == false);
    v = 1;
    QVERIFY(v.asBool() == true);
    v = 2;
    QVERIFY(v.asBool() == true);

}

inline void VariantTestSet::numberAsBool(void) {
    Variant v(static_cast<lua_Number>(0.0f));
    QVERIFY(v.asBool() == false);
    v = static_cast<lua_Number>(1.0f);
    QVERIFY(v.asBool() == true);
    v = static_cast<lua_Number>(2.0f);
    QVERIFY(v.asBool() == true);
}

inline void VariantTestSet::stringAsBool(void) {
    Variant v("false");
    QVERIFY(v.asBool() == false);
    v = "true";
    QVERIFY(v.asBool() == true);
    v = "fuckface";
    QVERIFY(v.asBool() == false);
}

inline void VariantTestSet::voidPtrAsBool(void) {
    Variant v((void*)this);
    QVERIFY(v.asBool() == true);

    v = (void*)nullptr;
    QVERIFY(v.asBool() == false);
}

inline void VariantTestSet::nilAsNumber(void) {
    Variant v;
    QVERIFY(v.asNumber() == 0);
}

inline void VariantTestSet::boolAsNumber(void) {
    Variant v(false);
    QVERIFY(v.asNumber() == 0);
    v = true;
    QVERIFY(v.asNumber() == 1);
}

inline void VariantTestSet::numberAsNumber(void) {
    Variant v(static_cast<lua_Number>(3.31f));
    QVERIFY(v.asNumber() == static_cast<lua_Number>(3.31f));
    v = static_cast<lua_Number>(14.0f);
    QVERIFY(v.asNumber() == static_cast<lua_Number>(14.0f));
}

inline void VariantTestSet::integerAsNumber(void) {
    Variant v(3);
    QVERIFY(v.asNumber() == static_cast<lua_Number>(3.0f));
    v = 13;
    QVERIFY(v.asNumber() == static_cast<lua_Number>(13.0f));
}

inline void VariantTestSet::numberAsInteger(void) {
    Variant v(static_cast<lua_Number>(3.31f));
    QVERIFY(v.asInteger() == 3);
    v = static_cast<lua_Number>(14.0f);
    QVERIFY(v.asInteger() == 14);
}

inline void VariantTestSet::stringAsNumber(void) {
    Variant v("3.2");
    QVERIFY(v.asNumber() == static_cast<lua_Number>(3.2f));
    v = "12";
    QVERIFY(v.asNumber() == static_cast<lua_Number>(12.0f));
}

inline void VariantTestSet::stringAsInteger(void) {
    Variant v("3");
    Variant intVariant = v.asInteger();
    QVERIFY(intVariant == 3);
    v = "12";
    QVERIFY(v.asInteger() == 12);
}

inline void VariantTestSet::voidPtrAsNumber(void) {
    Variant v((void*)this);
    QVERIFY(v.asNumber() == 1.0f);
    v = (void*)nullptr;
    QVERIFY(v.asNumber() == 0.0f);
}

inline void VariantTestSet::nilAsInteger(void) {
    Variant v;
    QVERIFY(v.asInteger() == 0);
}

inline void VariantTestSet::boolAsInteger(void) {
    Variant v(false);
    QVERIFY(v.asInteger() == 0);
    v = true;
    QVERIFY(v.asInteger() == 1);
}

inline void VariantTestSet::integerAsInteger(void) {
    Variant v(12);
    QVERIFY(v.asInteger() == 12);
    v = 34;
    QVERIFY(v.asInteger() == 34);
}

inline void VariantTestSet::voidPtrAsInteger(void) {
    Variant v((void*)this);
    QVERIFY(v.asInteger() == 1);
    v = (void*)nullptr;
    QVERIFY(v.asInteger() == 0);
}

inline void VariantTestSet::nilAsString(void) {
    Variant v;
    QVERIFY(v.asString() == "nil");
}

inline void VariantTestSet::boolAsString(void) {
    Variant v(false);
    QVERIFY(v.asString() == "false");
    v = true;
    QVERIFY(v.asString() == "true");
}

inline void VariantTestSet::numberAsString(void) {
    Variant v(static_cast<lua_Number>(12.5f));

    QVERIFY(QVariant(v.asString().getValue<const char*>()).value<lua_Number>()
            == static_cast<lua_Number>(12.5f)); //helps with inaccuracies and different precisions of strings!!!
    v = static_cast<lua_Number>(46);
    QString newStr = v.asString().getValue<const char*>();
    QVERIFY(newStr == "46.0");
}

inline void VariantTestSet::integerAsString(void) {
    Variant v(12);
    QVERIFY(QVariant(v.asString().getValue<const char*>()).value<lua_Integer>() == 12); //helps with inaccuracies and different precisions of strings!!!
    v = 46;
    QVERIFY(v.asString() == "46");
}

inline void VariantTestSet::stringAsString(void) {
    Variant v("Jesus");
    QVERIFY(v.asString() == "Jesus");
}

inline void VariantTestSet::voidPtrAsString(void) {
    Variant v((void*)this);

    v.push();

    QVERIFY(strcmp(v.asString().getValue<const char*>(), luaL_tolstring(thread(), -1, nullptr)) == 0);

    lua_pop(thread(), 2);
}

inline void VariantTestSet::voidPtrAsVoidPtr(void) {
    Variant v((void*)this);
    QVERIFY(v.asUserdata() == (void*)this);
}

inline void VariantTestSet::otherAsVoidPtr(void) {
    Variant v;
    QVERIFY(v.asUserdata() == nullptr);
}

inline void VariantTestSet::callCppLambdaClosure(void) {
#if 0
    int a = 3;
    Variant v;

    v.setValue(std::function<int(int)>([=](int b) {
        return a*b;
    }));

    int val = v(3).getValue<int>();
    QVERIFY(val == 9);
#endif
}

inline void VariantTestSet::makeWeakRef(void) {
#if 0
    elysian::InstanceTransform transform;
    Variant v1 = &transform;
    Variant v2 = Variant::makeWeakRef(v1);

    QVERIFY(v1 == v2);
    QVERIFY(v1.getValue<elysian::InstanceTransform*>() == v2.getValue<elysian::InstanceTransform*>());
    QVERIFY(v1.getRef() == v2.getRef());
#endif
}

inline void VariantTestSet::getLength(void) {
    Variant v1 = Variant::fromArray( {1, 2, 3, 4, 5 });
    Variant v2 = {
        { 1, 1 },
        { 2, 2 },
        { 3, 3 },
        { 6, 6 },
        {"LOLOL", "TEST" }
    };

    QVERIFY(v1.getLength() == 5);
    QVERIFY(v2.getLength() == 3);
}

inline void VariantTestSet::callLuaFunctionIntReturn(void) {
#if 0
    Variant variant;
    QVERIFY(LuaManager::runString(_luaFunc));
    QVERIFY(variant.pull());
    int i = variant(3, 4).getValue<int>();
    QVERIFY(i == 7);
#endif
}


inline void VariantTestSet::callLuaFunctionVariadicArgsVoidReturn(void) {
#if 0
    Variant v;
    thread().getGlobalsTable("print");
    QVERIFY(v.pull());
    QVERIFY(v("FUCK", "YOU", 3, 45.0f, "LOLZ").isNil());
#endif
}


inline void VariantTestSet::iterateTable(void) {
#if 0
    Variant v({{1,1}, {2,2}, {3,3}, {4,4}, {5,5}});
    int startVal = 1;
    int values = 0;

    for(auto it = v.begin(); it != v.end(); ++it) {
        QVERIFY(it.first() == values+1);
        QVERIFY(it.second() == startVal++);
        ++values;
    }
    QVERIFY(values == v.getLength());
#endif

}

inline void VariantTestSet::iterateTableConst(void) {
#if 0
    Variant o;
    const Variant v({{1,1}});
    int startVal = 1;
    int values = 0;

    for(auto it = v.begin(); it != v.end(); it++) {
        QVERIFY(it.first() == values+1);
        it.first().push();
        o.pull();
        QVERIFY(it.second() == startVal++);
        ++values;
    }
    QVERIFY(values == v.getLength());

#endif
}

inline void VariantTestSet::iterateTableCConst(void) {
#if 0
    const Variant v({{1,1}, {2,2}, {3,3}, {4,4}, {5,5}});
    int startVal = 1;
    int values = 0;

    for(auto it = v.cbegin(); it != v.cend(); ++it) {
        QVERIFY(it.first() == values+1);
        QVERIFY(it.second() == startVal++);
        ++values;
    }
    QVERIFY(values == v.getLength());
#endif
}

inline void VariantTestSet::iterateTableRangedBasedFor(void) {
#if 0
    std::initializer_list<elysian::LuaPair> array = { {1,2}, {2,3}, {3,5}, {4,7}, {5,8}, {6,10}, {7,12}, {8,33}};
    const Variant v(array);
    size_t values = 0;

    auto listIt = array.begin();
    for(auto it : v) {
        QVERIFY(*it == (*listIt).second);
        ++values;
        ++listIt;
    }
    QVERIFY(values == array.size());
#endif
}

inline void VariantTestSet::iterateTableModifyFields(void) {
#if 0
    Variant v({{1,1}, {2,2}, {3,3}, {4,4}, {5,5}});
    std::initializer_list<elysian::LuaPair> array = { {1,2}, {2,3}, {3,5}, {4,7}, {5,8}};

    auto listIt = array.begin();
    for(auto it = v.begin(); it != v.end(); ++it) {
        it->setVariant((*listIt).second);
        ++listIt;
    }

    listIt = array.begin();
    for(auto it : v) {
        QVERIFY(*it == (*listIt).second);
        ++listIt;
    }
#endif

}

}


#endif // TEST_LUA_VARIANT_H

