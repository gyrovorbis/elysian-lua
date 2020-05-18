#ifndef ELYSIAN_LUA_THREAD_VIEW_BASE_HPP
#define ELYSIAN_LUA_THREAD_VIEW_BASE_HPP

#include <functional>
#include <array>
#include <cassert>

extern "C" {
#   include <lua/lua.h>
#   include <lua/lualib.h>
#   include <lua/lauxlib.h>
}

#include <ElysianLua/elysian_lua_forward_declarations.hpp>
//#include "elysian_lua_stack.hpp"

#define ELYSIAN_LUA_PUSH_LITERAL(T, L) \
    lua_pushliteral(T, L)

#define ELYSIAN_LUA_ERROR_BUFFER_SIZE 512

namespace elysian::lua {

template <typename... Args>
struct LuaTableValues: public std::tuple<Args...> {
    LuaTableValues(Args&&... args):
        std::tuple<Args...>(args...){}
};

template<typename K, typename V>
struct LuaPair: public std::pair<K, V> {
    LuaPair(K key, V value):
        std::pair<K, V>(std::move(key), std::move(value)){}
};

template <typename, typename Enable = std::bool_constant<true>>
constexpr bool is_kvpair = false;

template <typename T>
constexpr bool is_kvpair<
    T, std::bool_constant<std::is_same_v<
           T, LuaPair<typename T::first_type, typename T::second_type>>>> =
    true;


template <typename Tuple> struct is_table_pairs_t;

template <typename... Us>
struct is_table_pairs_t<std::tuple<Us...>>
    : std::conjunction<std::bool_constant<is_kvpair<Us>>...> {};

template <typename Tuple>
constexpr bool is_table_pairs = is_table_pairs_t<Tuple>::value;

// static_assert(has_type<std::tuple<int, float, char *>>::value, "");

static_assert(is_kvpair<LuaPair<int, float>>, "");

static_assert(
    is_table_pairs<std::tuple<LuaPair<int, float>, LuaPair<int, float>,
                              LuaPair<int, float>>>,
    "");


struct StackRecord {
    int totalUsed = 0; // Total number of items consumed/produced
    int lastUsed = 0; // Number of items consumed/produced by last operation

    void use(int count);
};

inline void StackRecord::use(int count) {
    lastUsed = count;
    totalUsed += count;
}

class ThreadViewBase;

template<typename T>
bool stack_check(const ThreadViewBase* pBase, StackRecord& record, int index) {
    return stack_impl::stack_checker<T>::check(pBase, record, index);
}

template<typename T>
auto stack_get(const ThreadViewBase* pBase, StackRecord& record, int index) {
    return stack_impl::stack_getter<T>::get(pBase, record, index);
}

template<typename T>
int stack_push(const ThreadViewBase* pBase, StackRecord& record, const T& value) {
    return stack_impl::stack_pusher<T>::push(pBase, record, value);
}

// Wrapper around lua_State
class ThreadViewBase {
    friend class LuaVM;
public:
    ThreadViewBase(lua_State* state);

    bool isValid(void) const;
    bool isValidIndex(int index) const;
    lua_State *getState(void) const;

    // NEWFANGLED-ASS STACK API
    int getTop(void) const;
    void setTop(int index) const;
    int toAbsStackIndex(int index) const;
    int toRelativeStackIndex(int index) const;
    bool checkStack(unsigned slots) const;
    int setCStackLimit(unsigned int limit) const;
    lua_Number getVersion(void) const;
    void checkVersion(void) const;
    lua_CFunction atPanic(lua_CFunction panicf) const;
    lua_Alloc getAllocFunc(void **pUd) const;
    void setAllocFunc(lua_Alloc f, void* pUd) const;

    operator lua_State *(void)const;

    template<typename K>
    GlobalsTableProxy<K> operator[](K key);

    int push(void) const; // doesn't do shit
    int push(std::nullptr_t) const;
    int push(lua_Integer integer) const;
    int push(lua_Number number) const;
    int push(bool boolean) const;
    const char* push(const char* pString) const; // gracefully push nil for nullptr? Or empty string? wtf?
#ifdef ELYSIAN_LUA_USE_STD_STRING
    const char* push(std::string cppStr) const;
#endif
    int push(void* pLightUd) const; // gracefully push nil for nullptr
    int push(const lua_CFunction pCFunc) const; //gracefully push nil for nullptr

    template<typename T>
    int push(const T& value) const;

    template<typename... Args>
    int pushMulti(Args&&... args) const;

    const char* pushStringFormatted(const char *pFmt, ...) const;
    const char* pushStringVaList(const char *pFmt, va_list vaList) const;
    //const char* pushStringLiteral(const char* pLiteral); MAYBE MACRO
    const char* pushStringBuffer(const char* pBuff, size_t length) const;
    void pushValue(int index) const;
    void pushNil(void) const;
    bool pushThread(void) const;
    void pushCClosure(const lua_CFunction fn, int upvalueCount) const;
    void pushNewTable(int arraySize=0, int hashSize=0) const;
    void* pushNewUserDataUV(size_t size, int nuvalue) const;
    lua_State* pushNewThread(void) const;
    void pushLength(int index) const;
    lua_Integer length(int index) const;
    lua_Unsigned lengthRaw(int index) const;
    const char* pushAsString(int index, size_t* pLength=nullptr);

    StackTable createTable(int arraySize=0, int hashSize=0) const;

    template<typename T>
    std::enable_if_t<stack_impl::stack_table_type<T>, StackTable>
    createTable(const T& table) const;

    int ref(int index=LUA_REGISTRYINDEX) const;
    void unref(int ref) const;
    void unref(int table, int ref) const;

    void pop(int count=1) const;
    void remove(int index) const;
    void remove(int index, int count) const;
    void copy(int fromIndex, int toIndex) const;
    void xmove(lua_State* pTo, int n) const;
    void insert(int index) const;
    void rotate(int index, int n) const;
    int compare(int lhsIndex, int rhsIndex, int op) const;
    bool rawEqual(int index1, int index2) const;
    void arith(int op) const;
    void concat(int n) const;
    const char* gsub(const char* pSrc, const char* pSubStr, const char* pNewSubStr) const;

    int getType(int index) const;
    const char* getTypeName(int index) const;

    lua_Integer toInteger(int index, bool* pIsNum=nullptr) const;
    const char* toString(int index, size_t* pLen=nullptr) const;
    lua_Number toNumber(int index, bool* pIsNum=nullptr) const;
    bool toBoolean(int index) const;
    lua_CFunction toCFunction(int index) const;
    void toClose(int index) const;
    const void* toPointer(int index) const;
    lua_State* toThread(int index) const;
    void* toUserdata(int index) const;
    template <typename T> auto toValue(int index) const;

    // Conversion utilities
    size_t stringToNumber(const char* pString);
    static const char* getStatusString(int statusCode);
    static int numberToInteger(lua_Number n, lua_Integer* p);

    bool isInteger(int index) const;
    bool isBoolean(int index) const;
    bool isCFunction(int index) const;
    bool isFunction(int index) const;
    bool isLightUserdata(int index) const;
    bool isNil(int index) const;
    bool isNone(int index) const;
    bool isNoneOrNil(int index) const;
    bool isNumber(int index) const;
    bool isString(int index) const;
    bool isTable(int index) const;
    bool isThread(int index) const;
    bool isUserdata(int index) const;
    bool isObject(int index) const;

    int getStatus(void) const;
    bool isYieldable(void) const;

    GlobalsTable getGlobalsTable(void);
    void pushGlobalsTable(void) const;
    template<typename K>
    int getGlobalsTable(K&& key) const;
    template<typename K, typename V>
    int getGlobalsTable(K&& key, V& value) const;
    template<typename K>
    void setGlobalsTable(K&& key) const;
    template<typename K, typename V>
    void setGlobalsTable(K&& key, V&& value) const;

    template<typename First, typename... Rest>
    void appendTable(int tableIndex, const LuaTableValues<First, Rest...>& tableValues) const;
    template<typename First, typename... Rest>
    void appendTableRaw(int tableIndex, const LuaTableValues<First, Rest...>& tableValues) const;

    template<typename C, std::size_t... Is>
    void appendSequence(int index, const C& container, std::index_sequence<Is...>) const;
    template<typename C, std::size_t... Is>
    void appendSequenceRaw(int index, const C& container, std::index_sequence<Is...>) const;

    void setTable(int index) const;
    template<typename K, typename V>
    void setTable(int index, K&& key, V&& value) const;

    void setTableRaw(int index) const;
    template<typename K, typename V>
    void setTableRaw(int index, K&& key, V&& value) const;

    template<typename V, typename... Keys>
    void setTableMulti(int index, const std::tuple<Keys...>& keys, V&& value) const;
    template<typename V, typename C, std::size_t... Is>
    void setTableMulti(int index, const C& container, std::index_sequence<Is...>, V&& value) const;

    int getTable(int index) const;
    template<typename K>
    int getTable(int index, K&& key) const;
    template<typename K, typename V>
    int getTable(int index, const K& key, V& value) const;

    int getSubTable(int index, const char* pName) const;

    template<typename... Keys>
    int getTableMulti(int index, const std::tuple<Keys...>& keys) const;
    template<typename C, std::size_t... Is>
    int getTableMulti(int index, const C& container, std::index_sequence<Is...>) const;

    int getTableRaw(int index) const;
    template<typename K>
    int getTableRaw(int index, const K& key) const;
    template<typename K, typename V>
    int getTableRaw(int index, const K& key, V& value) const;

    int setUserValue(int index, int n) const;
    template<typename V>
    int setUserValue(int index, int n, const V& value) const;

    int getUserValue(int index, int n) const;
    template<typename V>
    int getUserValue(int index, int n, V& value) const;

    template<typename T>
    constexpr static int getType(void);

    template<typename T>
    bool checkType(int index) const;

    template<typename T>
    bool pull(T& value) const;

    int next(int index) const;

    template<typename F>
    auto iterateTable(int index, F&& body) const;

    void rPrint(int index, unsigned maxDepth=99, const char* pLabel="table");
    void rPrintCStack(int startIndex=-1, int endIndex=0, unsigned maxDepth=3, const char* pLabel="CStack");
    void close(void) const;
    int reset(void) const;
    int yield(int nResults) const;
    int yieldK(int nResults, lua_KContext ctx, lua_KFunction k);

    void call(int nargs, int nresults) const;
    void callK(int nargs, int nresults, lua_KContext ctx, lua_KFunction k) const;
    //Standard Lua pcall (msgh == 0 => return error, msgh != 0 => use as error-handler index)
    int pCall(int nargs, int nresults, int msgh) const;
    //Uses builtin ES error handler
    int pCall(int nargs, int nresults) const;
    int pCallK(int nargs, int nresults, int msgh, lua_KContext ctx, lua_KFunction k) const;
    int error(void) const;
    int error(const char *pFmt, ...) const;
    void setWarnFunc(lua_WarnFunction f, void* pUd) const;
    void warning(const char* pMsg, int toCont) const;
    int resume(lua_State *pFrom, int nargs, int* pnResults);

    int load(lua_Reader reader, void* pData, const char* pChunkName, const char* pMode) const;
    int loadString(const char* pStr) const;
    int loadBuffer(const char* pBuffer, size_t size, const char* pChunkName, const char* pMode=nullptr) const;
    int loadFile(const char* pFileName, const char* pMode=nullptr) const;
    int doString(const char* pString) const;
    int doFile(const char* pFileName) const;
    void registerFunc(const char* pName, lua_CFunction func) const;
    int dump(lua_Writer writer, void* pData, int strip) const;
    void* getExtraSpace(void) const;

    int setMetaTable(int index) const;
    void setMetaTable(const char* pTypeName) const;
    int getMetaTable(int index) const;
    int getMetaTable(const char* pTypeName) const;
    int getMetaField(int objIndex, const char* pName) const;
    int callMetaMethod(int objIndex, const char* pName) const;

    template<typename T, T... Args>
    int gc(T args...) const;

    int gcCollect(void) const;
    int gcStop(void) const;
    int gcRestart(void) const;
    int gcStep(int stepSize) const;

    //push/pop semantics for these?
    int gcIncremental(int pause, int stepmul, int stepSize) const;
    int gcGenerational(int minorMul, int majorMul) const;

    int gcMemoryKBytes(void) const;
    int gcMemoryBytes(void) const;
    size_t gcMemoryTotalBytes(void) const;
    bool gcIsRunning(void) const;

    // Debug API
    lua_Hook getHook(void) const;
    void setHook(lua_Hook f, int mask, int count) const;
    int getHookCount(void) const;
    int getHookMask(void) const;
    int getInfo(const char *pWhat, lua_Debug* pAr) const;
    const char* getLocal(const lua_Debug* pAr, int n) const;
    int getStack(int level, lua_Debug* pAr) const;
    const char* getUpValue(int funcIndex, int n) const;
    const char* setUpValue(int funcIndex, int n) const;
    void* upValueId(int funcIndex, int n) const;
    void upValueJoin(int funcIndex1, int n1, int funcIndex2, int n2) const;
    const char* setLocal(const lua_Debug* pAr, int n) const;

    // Lua Utility Library
    void newLibrary(const luaL_Reg l[]) const;
    void newLibraryTable(const luaL_Reg l[]) const;
    void newMetaTable(const char* pTypeName) const;
    void openStandardLibraries(void) const;
    void requireFunc(const char* pModuleName, lua_CFunction openFunc, int glb) const;
    void setTableFunctions(const luaL_Reg* pl, int nup) const;
    void* testUserData(int arg, const char* pTypeName) const;
    void traceBack(lua_State* pState, const char* pMessage, int level) const;
    void where(int level) const;

    template<typename T>
    T optionalValue(int index, T defaultValue) const;
    const char* optionalStringBuffer(int index, const char* defaultValue, size_t* pLength=nullptr) const;

    void argCheck(int cond, int arg, const char* pExtraMsg) const;
    int argError(int arg, const char* pExtraMsg) const;
    void argExpected(int cond, int arg, const char* pTypeName) const;
    void checkAny(int arg) const;
    lua_Integer checkInteger(int arg) const;
    lua_Number checkNumber(int argc) const;
    int checkOption(int arg, const char* pDefaultString, const char* const pValueStrings[]) const;
    void checkStack(int sz, const char* pMessage) const;
    const char* checkString(int arg, size_t* pSize=nullptr) const;
    void checkType(int arg, int t) const;
    void* checkUserData(int arg, const char* pTypeName) const;
    int typeError(int arg, const char* pTypeName) const;
    int execResult(int stat) const;
    int fileResult(int stat, const char* fName) const;

    // Buffer API
    void bufferInit(luaL_Buffer *pBuffer) const;
    char* bufferInitSize(luaL_Buffer* pBuffer, size_t size) const;
    char* bufferPrep(luaL_Buffer* pBuffer, size_t size=LUAL_BUFFERSIZE) const;


protected:
    ThreadViewBase(void) = default;
    void _setState(lua_State* pState);
    template<typename T>
    static const char* toCString(T&& complexStr);

protected:
      lua_State *m_pState = nullptr;
};


namespace internal {

    template<typename K>
    struct ThreadViewBaseTableHelperSetBase {
        template<typename V>
        static void set(const ThreadViewBase* pView, int index, const K& key, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(key);
            pView->push(value);
            lua_settable(pView->getState(), absIndex);
        }
    };

    template<typename K>
    struct ThreadViewBaseTableHelperSetRawBase {
        template<typename V>
        static void setRaw(const ThreadViewBase* pView, int index, const K& key, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(key);
            pView->push(value);
            lua_rawset(pView->getState(), absIndex);
        }
    };

    template<typename K>
    struct ThreadViewBaseTableHelperGetBase {
        static int get(const ThreadViewBase* pView, int index, const K& key) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(key);
            return lua_gettable(pView->getState(), absIndex);
        }
    };

    template<typename K>
    struct ThreadViewBaseTableHelperGetRawBase {
        static int getRaw(const ThreadViewBase* pView, int index, const K& key) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(key);
            return lua_rawget(pView->getState(), absIndex);
        }
    };

    template<typename K, typename Enable = void>
    struct ThreadViewBaseTableHelper:
        public ThreadViewBaseTableHelperGetBase<K>,
        public ThreadViewBaseTableHelperGetRawBase<K>,
        public ThreadViewBaseTableHelperSetBase<K>,
        public ThreadViewBaseTableHelperSetRawBase<K> {};

    template<typename I>
    struct ThreadViewBaseTableHelper<I,
             typename std::enable_if<std::is_integral<I>::value>::type> {

        static int get(const ThreadViewBase* pView, int index, I key) {
            if constexpr(!std::is_same_v<I, bool>) assert(key <= std::numeric_limits<lua_Integer>::max());
            return lua_geti(pView->getState(), index, static_cast<lua_Integer>(key));
        }

        static int getRaw(const ThreadViewBase* pView, int index, I key) {
            if constexpr(!std::is_same_v<I, bool>) assert(key <= std::numeric_limits<lua_Integer>::max());
            return lua_rawgeti(pView->getState(), index, static_cast<lua_Integer>(key));
        }

        template<typename V>
        static void set(const ThreadViewBase* pView, int index, I key, const V& value) {
            if constexpr(!std::is_same_v<I, bool>) assert(key <= std::numeric_limits<lua_Integer>::max());
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(value);
            lua_seti(pView->getState(), absIndex, static_cast<lua_Integer>(key));
        }

        template<typename V>
        static void setRaw(const ThreadViewBase* pView, int index, I key, const V& value) {
            if constexpr(!std::is_same_v<I, bool>) assert(key <= std::numeric_limits<lua_Integer>::max());
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(value);
            lua_rawseti(pView->getState(), absIndex, static_cast<lua_Integer>(key));
        }
    };

    template<>
    struct ThreadViewBaseTableHelper<const char*>:
            public ThreadViewBaseTableHelperGetRawBase<const char*>,
            public ThreadViewBaseTableHelperSetRawBase<const char*>
    {
        static int get(const ThreadViewBase* pView, int index, const char* pKey) {
            return lua_getfield(pView->getState(), index, pKey);
        }

        template<typename V>
        static void set(const ThreadViewBase* pView, int index, const char* pKey, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(value);
            lua_setfield(pView->getState(), absIndex, pKey);
        }
    };

    template<>
    struct ThreadViewBaseTableHelper<const void*>:
            public ThreadViewBaseTableHelperGetBase<const void*>,
            public ThreadViewBaseTableHelperSetBase<const void*>
    {
        static int getRaw(const ThreadViewBase* pView, int index, const void* pKey) {
            return lua_rawgetp(pView->getState(), index, pKey);
        }

        template<typename V>
        static void setRaw(const ThreadViewBase* pView, int index, const void* pKey, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(value);
            lua_setrawsetp(pView->getState(), absIndex, pKey);
        }
    };
}

template<typename T>
constexpr inline int ThreadViewBase::getType(void) { return LUA_TNONE; }
template<> constexpr inline int ThreadViewBase::getType<void>(void) { return LUA_TNONE; }
template<> constexpr inline int ThreadViewBase::getType<std::nullptr_t>(void) { return LUA_TNIL; }
template<> constexpr inline int ThreadViewBase::getType<void*>(void) { return LUA_TLIGHTUSERDATA; }
template<> constexpr inline int ThreadViewBase::getType<lua_Integer>(void) { return LUA_TNUMBER; }
template<> constexpr inline int ThreadViewBase::getType<lua_Number>(void) { return LUA_TNUMBER; }
template<> constexpr inline int ThreadViewBase::getType<bool>(void) { return LUA_TBOOLEAN; }
template<> constexpr inline int ThreadViewBase::getType<const char*>(void) { return LUA_TSTRING; }
#ifdef ELYSIAN_LUA_USE_STD_STRING
template<> constexpr inline int ThreadViewBase::getType<std::string>(void) { return LUA_TSTRING; }
#endif
template<> constexpr inline int ThreadViewBase::getType<lua_CFunction>(void) { return LUA_TFUNCTION; }
//template<> constexpr inline int ThreadViewBase::getType<Function>(void) { return LUA_TFUNCTION; }
template<> constexpr inline int ThreadViewBase::getType<lua_State*>(void) { return LUA_TTHREAD; }

template<typename T>
inline const char* ThreadViewBase::toCString(T&& complexStr) {
#ifdef ELYSIAN_LUA_USE_STD_STRING
    if constexpr(std::is_same_v<std::decay<T>::type, std::string>)
        return complexStr.c_str();
    else
#endif
    return complexStr;
}

inline const char* ThreadViewBase::getStatusString(int statusCode) {
    switch(statusCode) {
    case LUA_OK: return "Ok";
    case LUA_ERRRUN: return "Runtime Error";
    case LUA_ERRMEM: return "Memory Error";
    case LUA_ERRERR: return "Message Handler Error";
    case LUA_ERRSYNTAX: return "Syntax Error";
    case LUA_YIELD: return "Yield";
    case LUA_ERRFILE: return "File Error";
    default: return "Unknown Error";
    }
}

inline int ThreadViewBase::numberToInteger(lua_Number n, lua_Integer* p) {
    return lua_numbertointeger(n, p);
}

inline ThreadViewBase::operator lua_State *(void)const { return m_pState; }

template<typename K>
inline GlobalsTableProxy<K> ThreadViewBase::operator[](K key) {
    return GlobalsTableProxy<K>(getGlobalsTable(), std::make_tuple(std::move(key)));
}

inline ThreadViewBase::ThreadViewBase(lua_State* pState): m_pState(pState) {}

inline int ThreadViewBase::getTop(void) const { return lua_gettop(m_pState); }
inline void ThreadViewBase::setTop(int index) const { lua_settop(m_pState, index); }

inline lua_State *ThreadViewBase::getState(void) const { return m_pState; }

inline bool ThreadViewBase::isValid(void) const { return m_pState != nullptr; }
inline bool ThreadViewBase::isValidIndex(int index) const {
    return (isValid() && toAbsStackIndex(index) <= getTop());
}

inline void ThreadViewBase::_setState(lua_State *pState) { m_pState = pState; }

inline int ThreadViewBase::push(void) const { return 0; }
inline int ThreadViewBase::push(std::nullptr_t) const { pushNil(); return 1; }
inline void ThreadViewBase::pushNil(void) const { lua_pushnil(m_pState); }
inline int ThreadViewBase::push(lua_Integer integer) const { lua_pushinteger(m_pState, integer); return 1; }
inline int ThreadViewBase::push(lua_Number number) const { lua_pushnumber(m_pState, number); return 1; }
inline int ThreadViewBase::push(bool boolean) const { lua_pushboolean(m_pState, boolean); return 1;}
inline const char* ThreadViewBase::push(const char* pString) const {
    const char* pRetStr = nullptr;
    if(pString) pRetStr = lua_pushstring(m_pState, pString);
    else pushNil();
    return pRetStr;
}
#ifdef ELYSIAN_LUA_USE_STD_STRING
inline const char* ThreadViewBase::push(std::string cppStr) const {
    return lua_pushstring(m_pState, cppStr.c_str());
}
#endif
inline int ThreadViewBase::push(void* pLightUd) const {
    if(pLightUd) lua_pushlightuserdata(m_pState, pLightUd);
    else lua_pushnil(m_pState);
    return 1;
}
inline int ThreadViewBase::push(const lua_CFunction pCFunc) const {
    if(pCFunc) lua_pushcfunction(m_pState, pCFunc);
    else lua_pushnil(m_pState);
    return 1;
}
inline bool ThreadViewBase::pushThread(void) const {
    return lua_pushthread(m_pState);
}
inline const char* ThreadViewBase::pushStringVaList(const char *pFmt, va_list vaList) const {
  return lua_pushvfstring(m_pState, pFmt, vaList);
}
inline const char* ThreadViewBase::pushStringBuffer(const char* pBuff, size_t length) const {
    return lua_pushlstring(m_pState, pBuff, length);
}
inline void ThreadViewBase::pushGlobalsTable(void) const { lua_pushglobaltable(m_pState); }
inline void ThreadViewBase::pushCClosure(const lua_CFunction fn, int upvalueCount) const {
    assert(upvalueCount < getTop());
    lua_pushcclosure(m_pState, fn, upvalueCount);
}
inline void ThreadViewBase::pushValue(int index) const {
    assert(toAbsStackIndex(index) <= getTop());
    lua_pushvalue(m_pState, index);
}
template<typename T>
inline int ThreadViewBase::push(const T& value) const {
    StackRecord record;
    return stack_push(this, record, value);
}

template<typename... Args>
inline int ThreadViewBase::pushMulti(Args&&... args) const {
    return (push(args) + ...);
}

inline bool ThreadViewBase::checkStack(unsigned slotCount) const {
    return lua_checkstack(m_pState, static_cast<int>(slotCount));
}
inline int ThreadViewBase::setCStackLimit(unsigned int limit) const {
    return lua_setcstacklimit(m_pState, limit);
}
inline void ThreadViewBase::pop(int count) const {
    assert(count <= getTop());
    lua_pop(m_pState, count);
}

inline void ThreadViewBase::remove(int index) const {
    lua_remove(m_pState, index);
}

inline void ThreadViewBase::remove(int index, int count) const {
    if(count == 1) {
        remove(index);
    } else {
        const int firstIndex = toAbsStackIndex(index);
        const int lastIndex = firstIndex + (count - 1);
        assert(count > 0);
        assert(lastIndex <= getTop());
        rotate(firstIndex, getTop() - lastIndex);
        pop(count);
    }
}

inline void ThreadViewBase::copy(int fromIndex, int toIndex) const {
    lua_copy(m_pState, fromIndex, toIndex);
}

inline void ThreadViewBase::xmove(lua_State* pTo, int n) const {
    lua_xmove(m_pState, pTo, n);
}

inline void ThreadViewBase::insert(int index) const {
    lua_insert(m_pState, index);
}

inline void ThreadViewBase::rotate(int index, int n) const {
    assert(index <= getTop());
    assert(abs(n) <= getTop() - index);
    lua_rotate(m_pState, index, n);
}

inline int ThreadViewBase::compare(int lhsIndex, int rhsIndex, int op) const {
    return lua_compare(m_pState, lhsIndex, rhsIndex, op);
}

inline bool ThreadViewBase::rawEqual(int index1, int index2) const {
    return lua_rawequal(m_pState, index1, index2);
}

inline void ThreadViewBase::arith(int op) const { lua_arith(m_pState, op); }
inline void ThreadViewBase::concat(int n) const { lua_concat(m_pState, n); }
inline const char* ThreadViewBase::gsub(const char* pSrc, const char* pSubStr, const char* pNewSubStr) const { return luaL_gsub(m_pState, pSrc, pSubStr, pNewSubStr); }

inline int ThreadViewBase::getType(int index) const {
    assert(toAbsStackIndex(index) <= getTop());
    return lua_type(m_pState, index);
}
inline const char* ThreadViewBase::getTypeName(int index) const {
    return lua_typename(m_pState, getType(index));
}
inline int ThreadViewBase::toAbsStackIndex(int index) const {
    return lua_absindex(m_pState, index);
}
inline bool ThreadViewBase::isInteger(int index) const {
    assert(toAbsStackIndex(index) <= getTop());
    return lua_isinteger(m_pState, index);
}

inline bool ThreadViewBase::isBoolean(int index) const { return lua_isboolean(m_pState, index); }
inline bool ThreadViewBase::isCFunction(int index) const { return lua_iscfunction(m_pState, index); }
inline bool ThreadViewBase::isFunction(int index) const { return lua_isfunction(m_pState, index); }
inline bool ThreadViewBase::isLightUserdata(int index) const { return lua_islightuserdata(m_pState, index); }
inline bool ThreadViewBase::isNil(int index) const { return lua_isnil(m_pState, index); }
inline bool ThreadViewBase::isNone(int index) const { return lua_isnone(m_pState, index); }
inline bool ThreadViewBase::isNoneOrNil(int index) const { return lua_isnoneornil(m_pState, index); }
inline bool ThreadViewBase::isNumber(int index) const { return lua_isnumber(m_pState, index); }
inline bool ThreadViewBase::isString(int index) const { return lua_isstring(m_pState, index); }
inline bool ThreadViewBase::isTable(int index) const { return lua_istable(m_pState, index); }
inline bool ThreadViewBase::isThread(int index) const { return lua_isthread(m_pState, index); }
inline bool ThreadViewBase::isUserdata(int index) const { return lua_isuserdata(m_pState, index); }
inline bool ThreadViewBase::isObject(int index) const { return isFunction(index) || isTable(index) || isUserdata(index) || isThread(index); }

inline lua_Integer ThreadViewBase::toInteger(int index, bool* pIsNum) const {
    int isNum;
    int retVal = lua_tointegerx(m_pState, index, &isNum);
    if(pIsNum) *pIsNum = static_cast<bool>(isNum);
    return retVal;
}

inline const char* ThreadViewBase::toString(int index, size_t* pLen) const {
    return lua_tolstring(m_pState, index, pLen);
}

inline lua_Number ThreadViewBase::toNumber(int index, bool* pIsNum) const {
    int isNum;
    lua_Number retVal = lua_tonumberx(m_pState, index, &isNum);
    if(pIsNum) *pIsNum = static_cast<bool>(isNum);
    return retVal;
}

inline bool ThreadViewBase::toBoolean(int index) const {
    return lua_toboolean(m_pState, index);
}

inline lua_CFunction ThreadViewBase::toCFunction(int index) const {
    return lua_tocfunction(m_pState, index);
}

inline void ThreadViewBase::toClose(int index) const {
    return lua_toclose(m_pState, index);
}

inline const void* ThreadViewBase::toPointer(int index) const {
    return lua_topointer(m_pState, index);
}

inline lua_State* ThreadViewBase::toThread(int index) const {
    return lua_tothread(m_pState, index);
}

inline void* ThreadViewBase::toUserdata(int index) const {
    return lua_touserdata(m_pState, index);
}

inline lua_Number ThreadViewBase::getVersion(void) const {
    return lua_version(m_pState);
}

inline void ThreadViewBase::checkVersion(void) const {
    return luaL_checkversion(m_pState);
}

inline lua_CFunction ThreadViewBase::atPanic(lua_CFunction panicf) const {
    return lua_atpanic(m_pState, panicf);
}

inline lua_Alloc ThreadViewBase::getAllocFunc(void **pUd) const { return lua_getallocf(m_pState, pUd); }

inline void ThreadViewBase::setAllocFunc(lua_Alloc f, void* pUd) const {
    lua_setallocf(m_pState, f, pUd);
}

inline size_t ThreadViewBase::stringToNumber(const char* pString) {
    return lua_stringtonumber(m_pState, pString);
}

template <typename T>
inline auto ThreadViewBase::toValue(int index) const {
    StackRecord record;
    return stack_get<T>(this, record, index);
}

inline int ThreadViewBase::getStatus(void) const { return lua_status(m_pState); }
inline bool ThreadViewBase::isYieldable(void) const { return lua_isyieldable(m_pState); }

template<typename K>
inline int ThreadViewBase::getGlobalsTable(K&& key) const {
    if constexpr(getType<std::decay<K>::type>() == LUA_TSTRING) {
        return lua_getglobal(m_pState, toCString(key));
    } else {
        pushGlobalsTable();
        const int tableRef = toAbsStackIndex(-1);
        const int retVal = getTable(-1, key);
        remove(tableRef);
        return retVal;
    }
}

template<typename K, typename V>
inline int ThreadViewBase::getGlobalsTable(K&& key, V& value) const {
    int retType = getGlobalsTable(key);
    pull(value);
    return retType;
}


template<typename K>
inline void ThreadViewBase::setGlobalsTable(K&& key) const {
    if constexpr(getType<K>() == LUA_TSTRING) {
        lua_setglobal(m_pState, toCString(key));
    } else { //slow and reested!!!
        const int oldTop = getTop();
        pushGlobalsTable();
        push(key);
        pushValue(-3);
        getTable(-3);
        copy(oldTop);
        pop(getTop() - oldTop);
    }

}

template<typename K, typename V>
inline void ThreadViewBase::setGlobalsTable(K&& key, V&& value) const {
    pushGlobalsTable();
    setTable(-1, key, value);
    pop(1);
}

template<typename C, std::size_t... Is>
inline void ThreadViewBase::appendSequence(int index, const C& container, std::index_sequence<Is...>) const {
    (setTable(index, std::get<Is>(container).first, std::get<Is>(container).second), ...);
}

template<typename C, std::size_t... Is>
inline void ThreadViewBase::appendSequenceRaw(int index, const C& container, std::index_sequence<Is...>) const {
    (setTableRaw(index, std::get<Is>(container).first, std::get<Is>(container).second), ...);
}

template<typename First, typename... Rest>
inline void ThreadViewBase::appendTable(int index, const LuaTableValues<First, Rest...>& tableValues) const {
    appendSequence(index, tableValues, std::index_sequence_for<First, Rest...>());
}

template<typename First, typename... Rest>
inline void ThreadViewBase::appendTableRaw(int index, const LuaTableValues<First, Rest...>& tableValues) const {
    appendSequenceRaw(index, tableValues, std::index_sequence_for<First, Rest...>());
}

inline void ThreadViewBase::setTable(int index) const {
    lua_settable(m_pState, index);
}

template<typename K, typename V>
inline void ThreadViewBase::setTable(int index, K&& key, V&& value) const {
    internal::ThreadViewBaseTableHelper<K>::set(this, index, key, value);
}

inline void ThreadViewBase::setTableRaw(int index) const {
    lua_rawset(m_pState, index);
}

template<typename K, typename V>
inline void ThreadViewBase::setTableRaw(int index, K&& key, V&& value) const {
    internal::ThreadViewBaseTableHelper<K>::setRaw(this, index, key, value);
}

inline int ThreadViewBase::getTable(int index) const {
    return lua_gettable(m_pState, index);
}

inline int ThreadViewBase::getTableRaw(int index) const {
    return lua_rawget(m_pState, index);
}

template<typename T>
inline bool ThreadViewBase::checkType(int index) const {
    StackRecord record;
    return stack_check<T>(this, record, index);
}

template<typename V>
inline bool ThreadViewBase::pull(V& value) const {
    static_assert(stack_impl::stack_pull_pop_count<V>, "Cannot use pull semantics with stack objects!");

    bool success = false;
    const int startIndex = -stack_impl::stack_count<V>;

    if(checkType<V>(startIndex)) {
        value = toValue<V>(startIndex);
        success = true;
        pop(stack_impl::stack_pull_pop_count<V>);
    }
    return success;
}

template<typename K, typename V>
inline int ThreadViewBase::getTable(int index, const K& key, V& value) const {
    const int retType = internal::ThreadViewBaseTableHelper<K>::get(this, index, key);
    pull(value);
    return retType;
}

template<typename K>
inline int ThreadViewBase::getTable(int index, K&& key) const {
    return internal::ThreadViewBaseTableHelper<K>::get(this, index, key);
}

template<typename K, typename V>
inline int ThreadViewBase::getTableRaw(int index, const K& key, V& value) const {
    const int retType = internal::ThreadViewBaseTableHelper<K>::getRaw(this, index, key);
    pull(value);
    return retType;
}

template<typename K>
inline int ThreadViewBase::getTableRaw(int index, const K& key) const {
    return internal::ThreadViewBaseTableHelper<K>::getRaw(this, index, key);
}

inline int ThreadViewBase::getSubTable(int index, const char* pName) const {
    return luaL_getsubtable(m_pState, index, pName);
}

inline int ThreadViewBase::setUserValue(int index, int n) const {
    return lua_setiuservalue(m_pState, index, n);
}

template<typename V>
inline int ThreadViewBase::setUserValue(int index, int n, const V& value) const {
    push(value);
    return setUserValue(index, n);
}

inline int ThreadViewBase::getUserValue(int index, int n) const {
    return lua_getiuservalue(m_pState, index, n);
}

template<typename V>
inline int ThreadViewBase::getUserValue(int index, int n, V& value) const {
    int retVal = getUserValue(n);
    pull(value);
    return retVal;
}

//CHANGE ME TO USE ROTATE!!!
template<typename... Keys>
inline int ThreadViewBase::getTableMulti(int index, const std::tuple<Keys...>& keys) const {
    return getTableMulti(index, keys, std::index_sequence_for<Keys...>());
}

template<typename C, std::size_t... Is>
inline int ThreadViewBase::getTableMulti(int index, const C& container, std::index_sequence<Is...>) const {
    const int absIndex = toAbsStackIndex(index);
    const int top = getTop();

    auto processElement = [&](const C& cont, auto Idx) {
        /* Compile-time shenanigans to avoid having to duplicate the source
         * table before this loop. */
        constexpr const int currentIndex = (Idx == 0)? absIndex : -1;
        return getTable(currentIndex, std::get<Idx>(cont));
    };

    int retVal = (processElement(container, std::integral_constant<std::size_t, Is>()), ...);

    insert(top + 1);
    pop(std::index_sequence<Is...>::size() - 1);
    assert(getTop() == top + 1);
    return retVal;
}


template<typename V, typename... Keys>
inline void ThreadViewBase::setTableMulti(int index, const std::tuple<Keys...>& keys, V&& value) const {
    setTableMulti(index, keys, std::index_sequence_for<Keys...>(), std::forward<V>(value));
}

template<typename V, typename C, std::size_t... Is>
inline void ThreadViewBase::setTableMulti(int index, const C& container, std::index_sequence<Is...>, V&& value) const {
    const int absIndex = toAbsStackIndex(index);

    auto processElement = [&](const C& cont, auto Idx) {
        /* Compile-time shenanigans to avoid having to duplicate the source
         * table before this loop. */
        constexpr const int currentIndex = (Idx == 0)? absIndex : -1;

        if constexpr(Idx == std::index_sequence<Is...>::size()) {
            setTable(currentIndex, std::get<Idx>(cont), std::forward<V>(value));
        } else {
            getTable(currentIndex, std::get<Idx>(cont));
        }
    };

    (processElement(container, std::integral_constant<std::size_t, Is>()), ...);
    pop(std::index_sequence<Is...>::size() - 1);
}

inline void ThreadViewBase::close(void) const {
    lua_close(m_pState);
}

inline int ThreadViewBase::reset(void) const {
    return lua_resetthread(m_pState);
}

inline int ThreadViewBase::yield(int nResults) const {
    return lua_yield(m_pState, nResults);
}

inline int ThreadViewBase::yieldK(int nResults, lua_KContext ctx, lua_KFunction k) {
    return lua_yieldk(m_pState, nResults, ctx, k);
}

inline void ThreadViewBase::call(int nargs, int nresults) const { lua_call(m_pState, nargs, nresults); }
inline void ThreadViewBase::callK(int nargs, int nresults, lua_KContext ctx, lua_KFunction k) const {
    lua_callk(m_pState, nargs, nresults, ctx, k);
}

inline int ThreadViewBase::pCall(int nargs, int nresults, int msgh) const {
    assert(getTop() >= nargs + 1); // There's no way there's enough values on the stack...
    assert(getType(getTop() - nargs) == LUA_TFUNCTION); //Expected a function!
    assert(msgh == 0 || getType(msgh) == LUA_TFUNCTION); //Message handler isn't a function!

    return lua_pcall(m_pState, nargs, nresults, msgh);
}

inline int ThreadViewBase::pCall(int nargs, int nresults) const {
    //push error handler (or should we always keep this shit on the stack!?!
    return pCall(nargs, nresults, 0);
    //remove from stack?
}

inline int ThreadViewBase::pCallK(int nargs, int nresults, int msgh, lua_KContext ctx, lua_KFunction k) const {
    return lua_pcallk(m_pState, nargs, nresults, msgh, ctx, k);
}

inline int ThreadViewBase::error(void) const { return lua_error(m_pState); }

inline int ThreadViewBase::error(const char *pFmt, ...) const {
    va_list args;
    char buffer[ELYSIAN_LUA_ERROR_BUFFER_SIZE];
    va_start(args, pFmt);
    vsnprintf(buffer, sizeof(buffer), pFmt, args);
    va_end(args);
    return luaL_error(m_pState, buffer);
}

inline void ThreadViewBase::setWarnFunc(lua_WarnFunction f, void* pUd) const { lua_setwarnf(m_pState, f, pUd); }

inline void ThreadViewBase::warning(const char* pMsg, int toCont) const { lua_warning(m_pState, pMsg, toCont); }

inline int ThreadViewBase::resume(lua_State *pFrom, int nargs, int* pnResults) {
    return lua_resume(m_pState, pFrom, nargs, pnResults);
}

inline int ThreadViewBase::load(lua_Reader reader, void* pData, const char* pChunkName, const char* pMode) const {
    return lua_load(m_pState, reader, pData, pChunkName, pMode);
}

inline int ThreadViewBase::loadString(const char* pStr) const {
    return luaL_loadstring(m_pState, pStr);
}

inline  int ThreadViewBase::loadBuffer(const char* pBuffer, size_t size, const char* pChunkName, const char* pMode) const {
    return luaL_loadbufferx(m_pState, pBuffer, size, pChunkName, pMode);
}

inline int ThreadViewBase::loadFile(const char* pFileName, const char* pMode) const {
    return luaL_loadfilex(m_pState, pFileName, pMode);
}

inline int ThreadViewBase::doString(const char* pString) const { return luaL_dostring(m_pState, pString); }
inline int ThreadViewBase::doFile(const char* pFileName) const { return luaL_dofile(m_pState, pFileName); }

inline void ThreadViewBase::registerFunc(const char* pName, lua_CFunction func) const {
    lua_register(m_pState, pName, func);
}

inline int ThreadViewBase::dump(lua_Writer writer, void* pData, int strip) const {
    return lua_dump(m_pState, writer, pData, strip);
}

template<typename T, T... Args>
inline int ThreadViewBase::gc(T args...) const {
    static_assert(std::is_same_v<T, int>, "Arguments are all required to be integers!");
    return lua_gc(m_pState, args);
}
inline int ThreadViewBase::gcCollect(void) const { return gc(LUA_GCCOLLECT); }
inline int ThreadViewBase::gcStop(void) const { return gc(LUA_GCSTOP); }
inline int ThreadViewBase::gcRestart(void) const { return gc(LUA_GCRESTART); }
inline int ThreadViewBase::gcStep(int stepSize) const { return gc(LUA_GCSTEP, stepSize); }
inline int ThreadViewBase::gcIncremental(int pause, int stepMul, int stepSize) const {
    return gc(LUA_GCINC, pause, stepMul, stepSize);
}
inline int ThreadViewBase::gcGenerational(int minorMul, int majorMul) const {
    return gc(LUA_GCGEN, minorMul, majorMul);
}
inline int ThreadViewBase::gcMemoryKBytes(void) const { return gc(LUA_GCCOUNT); }
inline int ThreadViewBase::gcMemoryBytes(void) const  { return gc(LUA_GCCOUNTB); }
inline size_t ThreadViewBase::gcMemoryTotalBytes(void) const {
    return static_cast<size_t>(gcMemoryKBytes() * 1024 + gcMemoryBytes());
}
inline bool ThreadViewBase::gcIsRunning(void) const { return gc(LUA_GCISRUNNING); }

inline lua_Hook ThreadViewBase::getHook(void) const { return lua_gethook(m_pState); }
inline void ThreadViewBase::setHook(lua_Hook f, int mask, int count) const { return lua_sethook(m_pState, f, mask, count); }
inline int ThreadViewBase::getHookCount(void) const { return lua_gethookcount(m_pState); }
inline int ThreadViewBase::getHookMask(void) const { return lua_gethookmask(m_pState); }
inline int ThreadViewBase::getInfo(const char *pWhat, lua_Debug* pAr) const { return lua_getinfo(m_pState, pWhat, pAr); }
inline const char* ThreadViewBase::getLocal(const lua_Debug* pAr, int n) const { return lua_getlocal(m_pState, pAr, n); }
inline int ThreadViewBase::getStack(int level, lua_Debug* pAr) const { return lua_getstack(m_pState, level, pAr); }
inline const char* ThreadViewBase::getUpValue(int funcIndex, int n) const { return lua_getupvalue(m_pState, funcIndex, n); }
inline const char* ThreadViewBase::setUpValue(int funcIndex, int n) const { return lua_setupvalue(m_pState, funcIndex, n); }
inline void* ThreadViewBase::upValueId(int funcIndex, int n) const { return lua_upvalueid(m_pState, funcIndex, n); }
inline void ThreadViewBase::upValueJoin(int funcIndex1, int n1, int funcIndex2, int n2) const { return lua_upvaluejoin(m_pState, funcIndex1, n1, funcIndex2, n2); }
inline const char* ThreadViewBase::setLocal(const lua_Debug* pAr, int n) const { return lua_setlocal(m_pState, pAr, n); }
inline void ThreadViewBase::newLibrary(const luaL_Reg l[]) const { luaL_newlib(m_pState, l); }
inline void ThreadViewBase::newLibraryTable(const luaL_Reg l[]) const { luaL_newlibtable(m_pState, l); }
inline void ThreadViewBase::newMetaTable(const char* pTypeName) const { luaL_newmetatable(m_pState, pTypeName); }
inline void ThreadViewBase::openStandardLibraries(void) const { luaL_openlibs(m_pState); }
inline void ThreadViewBase::requireFunc(const char* pModuleName, lua_CFunction openFunc, int glb) const { luaL_requiref(m_pState, pModuleName, openFunc, glb); }
inline void ThreadViewBase::setTableFunctions(const luaL_Reg* pl, int nup) const { luaL_setfuncs(m_pState, pl, nup); }
inline void* ThreadViewBase::testUserData(int arg, const char* pTypeName) const { return luaL_testudata(m_pState, arg, pTypeName); }
inline void ThreadViewBase::traceBack(lua_State* pState, const char* pMessage, int level) const { luaL_traceback(m_pState, pState, pMessage, level); }
inline void ThreadViewBase::where(int level) const { luaL_where(m_pState, level); }

inline void ThreadViewBase::pushNewTable(int arraySize, int hashSize) const {
    lua_createtable(m_pState, arraySize, hashSize);
}

inline void* ThreadViewBase::pushNewUserDataUV(size_t size, int nuvalue) const {
    return lua_newuserdatauv(m_pState, size, nuvalue);
}

inline lua_State* ThreadViewBase::pushNewThread(void) const {
    return lua_newthread(m_pState);
}

inline void ThreadViewBase::pushLength(int index) const {
    lua_len(m_pState, index);
}

inline lua_Integer ThreadViewBase::length(int index) const { return luaL_len(m_pState, index); }

inline lua_Unsigned ThreadViewBase::lengthRaw(int index) const { return lua_rawlen(m_pState, index); }

inline const char* ThreadViewBase::pushAsString(int index, size_t* pLength) {
    return luaL_tolstring(m_pState, index, pLength);
}

inline int ThreadViewBase::ref(int index) const {
    return luaL_ref(m_pState, index);
}
inline void ThreadViewBase::unref(int table, int ref) const {
    luaL_unref(m_pState, table, ref);
}
inline void ThreadViewBase::unref(int ref) const {
    unref(LUA_REGISTRYINDEX, ref);
}

template<typename T>
inline std::enable_if_t<stack_impl::stack_table_type<T>, StackTable>
ThreadViewBase::createTable(const T& value) const {
    push(value);
    return toValue<StackTable>(-1);
}

inline int ThreadViewBase::next(int index) const {
    return lua_next(m_pState, index);
}

template<typename F>
inline auto ThreadViewBase::iterateTable(int index, F&& body) const {
    const int absIndex = toAbsStackIndex(index);
    pushNil();

    if constexpr(std::is_same_v<bool, std::invoke_result_t<F>>) {
        while(next(absIndex) != 0) {
            bool proceed = body();
            pop();
            if(!proceed) {
                pop();
                return false;
            }
        }
        return true;

    } else {
        while(next(absIndex) != 0) {
            body();
            pop();
        }
    }
}

inline int ThreadViewBase::setMetaTable(int index) const { return lua_setmetatable(m_pState, index); }
inline void ThreadViewBase::setMetaTable(const char* pTypeName) const { return luaL_setmetatable(m_pState, pTypeName); }
inline int ThreadViewBase::getMetaTable(int index) const { return lua_getmetatable(m_pState, index); }
inline int ThreadViewBase::getMetaTable(const char* pTypeName) const { return luaL_getmetatable(m_pState, pTypeName); }
inline int ThreadViewBase::getMetaField(int objIndex, const char* pName) const { return luaL_getmetafield(m_pState, objIndex, pName); }
inline int ThreadViewBase::callMetaMethod(int objIndex, const char* pName) const { return luaL_callmeta(m_pState, objIndex, pName); }
inline void* ThreadViewBase::getExtraSpace(void) const { return lua_getextraspace(m_pState); }

template<typename T>
inline T ThreadViewBase::optionalValue(int index, T defaultValue) const {
    return (lua_isnoneornil(m_pState, index)? defaultValue : toValue<T>(index));
}

template<>
inline lua_Integer ThreadViewBase::optionalValue<lua_Integer>(int index, lua_Integer defaultValue) const {
    return luaL_optinteger(m_pState, index, defaultValue);
}

template<>
inline lua_Number ThreadViewBase::optionalValue<lua_Number>(int index, lua_Number defaultValue) const {
    return luaL_optnumber(m_pState, index, defaultValue);
}

template<>
inline const char* ThreadViewBase::optionalValue<const char*>(int index, const char* defaultValue) const {
    return luaL_optstring(m_pState, index, defaultValue);
}

inline const char* ThreadViewBase::optionalStringBuffer(int index, const char* defaultValue, size_t* pLength) const {
    return luaL_optlstring(m_pState, index, defaultValue, pLength);
}

inline void ThreadViewBase::argCheck(int cond, int arg, const char* pExtraMsg) const { luaL_argcheck(m_pState, cond, arg, pExtraMsg); }
inline int ThreadViewBase::argError(int arg, const char* pExtraMsg) const { return luaL_argerror(m_pState, arg, pExtraMsg); }
inline void ThreadViewBase::argExpected(int cond, int arg, const char* pTypeName) const { luaL_argexpected(m_pState, cond, arg, pTypeName); }
inline void ThreadViewBase::checkAny(int arg) const { luaL_checkany(m_pState, arg); }
inline lua_Integer ThreadViewBase::checkInteger(int arg) const { return luaL_checkinteger(m_pState, arg); }
inline lua_Number ThreadViewBase::checkNumber(int arg) const { return luaL_checknumber(m_pState, arg); }
inline int ThreadViewBase::checkOption(int arg, const char* pDefaultString, const char* const pValueStrings[]) const { return luaL_checkoption(m_pState, arg, pDefaultString, pValueStrings); }
inline void ThreadViewBase::checkStack(int sz, const char* pMessage) const { luaL_checkstack(m_pState, sz, pMessage); }
inline const char* ThreadViewBase::checkString(int arg, size_t *pSize) const {return luaL_checklstring(m_pState, arg, pSize); }
inline void ThreadViewBase::checkType(int arg, int t) const { luaL_checktype(m_pState, arg, t); }
inline void* ThreadViewBase::checkUserData(int arg, const char* pTypeName) const { return luaL_checkudata(m_pState, arg, pTypeName); }
inline int ThreadViewBase::typeError(int arg, const char* pTypeName) const { return luaL_typeerror(m_pState, arg, pTypeName); }
inline int ThreadViewBase::execResult(int stat) const { return luaL_execresult(m_pState, stat); }
inline int ThreadViewBase::fileResult(int stat, const char* fName) const { return luaL_fileresult(m_pState, stat, fName); }

inline void ThreadViewBase::bufferInit(luaL_Buffer *pBuffer) const { luaL_buffinit(m_pState, pBuffer); }
inline char* ThreadViewBase::bufferInitSize(luaL_Buffer* pBuffer, size_t size) const { return luaL_buffinitsize(m_pState, pBuffer, size); }
inline char* ThreadViewBase::bufferPrep(luaL_Buffer* pBuffer, size_t size) const { return luaL_prepbuffsize(pBuffer, size); }

}

#endif // ELYSIAN_LUA_THREAD_VIEW_BASE_HPP
