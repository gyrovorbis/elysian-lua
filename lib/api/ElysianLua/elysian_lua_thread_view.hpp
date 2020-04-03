#ifndef ELYSIAN_LUA_THREAD_VIEW_HPP
#define ELYSIAN_LUA_THREAD_VIEW_HPP

#include <functional>
#include <array>
#include <cassert>

extern "C" {
    #include <lua/lua.h>
}

#include <ElysianLua/elysian_lua_forward_declarations.hpp>

#define ELYSIAN_LUA_PUSH_LITERAL(T, L) \
    lua_pushliteral(T, L)

namespace elysian::lua {

template <typename... Args> using LuaTableValues = std::tuple<Args...>;

template <typename K, typename V> using LuaKVPair = std::pair<K, V>;


template <typename, typename Enable = std::bool_constant<true>>
constexpr bool is_kvpair = false;

template <typename T>
constexpr bool is_kvpair<
    T, std::bool_constant<std::is_same_v<
           T, LuaKVPair<typename T::first_type, typename T::second_type>>>> =
    true;


template <typename Tuple> struct is_table_pairs_t;

template <typename... Us>
struct is_table_pairs_t<std::tuple<Us...>>
    : std::conjunction<std::bool_constant<is_kvpair<Us>>...> {};

template <typename Tuple>
constexpr bool is_table_pairs = is_table_pairs_t<Tuple>::value;

// static_assert(has_type<std::tuple<int, float, char *>>::value, "");

static_assert(is_kvpair<LuaKVPair<int, float>>, "");

static_assert(
    is_table_pairs<std::tuple<LuaKVPair<int, float>, LuaKVPair<int, float>,
                              LuaKVPair<int, float>>>,
    "");


// Wrapper around lua_State
class ThreadView {
    friend class LuaVM;
public:
    ThreadView(lua_State* state);

    bool isValid(void) const;
    bool isValidIndex(int index) const;
    lua_State *getState(void) const;

    // NEWFANGLED-ASS STACK API
    int getTop(void) const;
    int toAbsStackIndex(int index) const;
    int toRelativeStackIndex(int index) const;
    bool checkStack(unsigned slots) const;
    int setCStackLimit(unsigned int limit) const;

    operator lua_State *(void)const;

    void push(void) const; // doesn't do shit
    void push(std::nullptr_t) const;
    void push(lua_Integer integer) const;
    void push(lua_Number number) const;
    void push(bool boolean) const;
    const char* push(const char* pString) const; // gracefully push nil for nullptr? Or empty string? wtf?
#ifdef ELYSIAN_LUA_USE_STD_STRING
    const char* push(std::string cppStr) const;
#endif
    void push(void* pLightUd) const; // gracefully push nil for nullptr
    void push(const lua_CFunction pCFunc) const; //gracefully push nil for nullptr

    template<typename T, size_t S>
    void push(const std::array<T, S>& array) const;
    
    template<typename K, typename V>
    void push(const LuaKVPair<K, V>& pair) const;
    
    template<typename First, typename... Rest>
    void push(const LuaTableValues<First, Rest...>& tableValues) const;

    const char* pushStringFormatted(const char *pFmt, ...) const;
    const char* pushStringVaList(const char *pFmt, va_list vaList) const;
    //const char* pushStringLiteral(const char* pLiteral); MAYBE MACRO
    const char* pushStringBuffer(const char* pBuff, size_t length) const;
    void pushValue(int index) const;
    void pushNil(void) const;
    bool pushThread(void) const;
    void pushCClosure(const lua_CFunction fn, int upvalueCount) const;

    void pop(int count=1) const;
    void remove(int index) const;
    void copy(int fromIndex, int toIndex) const;
    void insert(int index) const;

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

    lua_Number getVersion(void) const;
    size_t stringToNumber(const char* pString);

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

    int getStatus(void) const;
    bool isYieldable(void) const;

    GlobalsTable getGlobalsTable(void);
    void pushGlobalsTable(void) const;
    template<typename K>
    int getGlobalsTable(K&& key) const;
    template<typename K, typename V>
    int getGlobalsTable(K&& key, V& value) const;
    template<typename K>
    void setGlobalsTable(K&& key);
    template<typename K, typename V>
    void setGlobalsTable(K&& key, V&& value);

    template <typename T> T toValue(int index) const;
    
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
    void setTable(int index, const K& key, const V& value) const;

    void setTableRaw(int index) const;
    template<typename K, typename V>
    void setTableRaw(int index, const K& key, const V& value) const;

    template<typename V, typename... Keys>
    void setTableMulti(int index, const std::tuple<Keys...>& keys, V&& value) const;
    template<typename V, typename C, std::size_t... Is>
    void setTableMulti(int index, const C& container, std::index_sequence<Is...>, V&& value) const;

    int getTable(int index) const;
    template<typename K>
    int getTable(int index, K&& key) const;
    template<typename K, typename V>
    int getTable(int index, const K& key, V& value) const;

    template<typename... Keys>
    int getTableMulti(int index, const std::tuple<Keys...>& keys) const;
    template<typename C, std::size_t... Is>
    int getTableMulti(int index, const C& container, std::index_sequence<Is...>) const;

    int getTableRaw(int index) const;
    template<typename K>
    int getTableRaw(int index, K&& key) const;
    template<typename K, typename V>
    int getTableRaw(int index, const K& key, V& value) const;

    template<typename T>
    constexpr static int getType(void);

    template<typename T>
    bool checkType(int index) const;

    template<typename T>
    bool pull(T& value) const;

    template<typename F>
    void iteratePairs(int index, F&& body) const;

    template<typename F>
    void iterateiPairs(int index, F&& body) const;

    void rPrint(int index, unsigned maxDepth=99, const char* pName="table");
    void close(void) const;
    bool reset(void);

    //push std::vector<>
    //push std::hash

    template<typename T>
    static const char* toCString(T&& complexStr);


protected:
    ThreadView(void) = default;
    void _setState(lua_State* pState);

private:
      lua_State *m_pState = nullptr;
};


namespace internal {

    template<typename K>
    struct ThreadViewTableHelperSetBase {
        template<typename V>
        static void set(const ThreadView* pView, int index, const K& key, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(key);
            pView->push(value);
            lua_settable(pView->getState(), absIndex);
        }
    };

    template<typename K>
    struct ThreadViewTableHelperSetRawBase {
        template<typename V>
        static void setRaw(const ThreadView* pView, int index, const K& key, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(key);
            pView->push(value);
            lua_rawset(pView->getState(), absIndex);
        }
    };

    template<typename K>
    struct ThreadViewTableHelperGetBase {
        static int get(const ThreadView* pView, int index, const K& key) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(key);
            return lua_gettable(pView->getState(), absIndex);
        }
    };

    template<typename K>
    struct ThreadViewTableHelperGetRawBase {
        static int getRaw(ThreadView* pView, int index, const K& key) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(key);
            return lua_rawget(pView->getState(), absIndex);
        }
    };

    template<typename K>
    struct ThreadViewTableHelper:
        public ThreadViewTableHelperGetBase<K>,
        public ThreadViewTableHelperGetRawBase<K>,
        public ThreadViewTableHelperSetBase<K>,
        public ThreadViewTableHelperSetRawBase<K> {};

    template<>
    struct ThreadViewTableHelper<int> {

        static int get(const ThreadView* pView, int index, int key) {
            return lua_geti(pView->getState(), index, key);
        }

        static int getRaw(const ThreadView* pView, int index, int key) {
            return lua_rawgeti(pView->getState(), index, key);
        }

        template<typename V>
        static void set(const ThreadView* pView, int index, int key, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(value);
            lua_seti(pView->getState(), absIndex, key);
        }

        template<typename V>
        static void setRaw(const ThreadView* pView, int index, int key, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(value);
            lua_rawseti(pView->getState(), absIndex, key);
        }
    };

    template<>
    struct ThreadViewTableHelper<const char*>:
            public ThreadViewTableHelperGetRawBase<const char*>,
            public ThreadViewTableHelperSetRawBase<const char*>
    {
        static int get(const ThreadView* pView, int index, const char* pKey) {
            return lua_getfield(pView->getState(), index, pKey);
        }

        template<typename V>
        static void set(const ThreadView* pView, int index, const char* pKey, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(value);
            lua_setfield(pView->getState(), absIndex, pKey);
        }
    };

    template<>
    struct ThreadViewTableHelper<const void*>:
            public ThreadViewTableHelperGetBase<const void*>,
            public ThreadViewTableHelperSetBase<const void*>
    {
        static int getRaw(const ThreadView* pView, int index, const void* pKey) {
            return lua_rawgetp(pView->getState(), index, pKey);
        }

        template<typename V>
        static void setRaw(const ThreadView* pView, int index, const void* pKey, const V& value) {
            const int absIndex = pView->toAbsStackIndex(index);
            pView->push(value);
            lua_setrawsetp(pView->getState(), absIndex, pKey);
        }
    };
}

template<typename T>
constexpr inline int ThreadView::getType(void) { return LUA_TNONE; }
template<> constexpr inline int ThreadView::getType<void>(void) { return LUA_TNONE; }
template<> constexpr inline int ThreadView::getType<nullptr_t>(void) { return LUA_TNIL; }
template<> constexpr inline int ThreadView::getType<void*>(void) { return LUA_TLIGHTUSERDATA; }
template<> constexpr inline int ThreadView::getType<lua_Integer>(void) { return LUA_TNUMBER; }
template<> constexpr inline int ThreadView::getType<lua_Number>(void) { return LUA_TNUMBER; }
template<> constexpr inline int ThreadView::getType<bool>(void) { return LUA_TBOOLEAN; }
template<> constexpr inline int ThreadView::getType<const char*>(void) { return LUA_TSTRING; }
#ifdef ELYSIAN_LUA_USE_STD_STRING
template<> constexpr inline int ThreadView::getType<std::string>(void) { return LUA_TSTRING; }
#endif
template<> constexpr inline int ThreadView::getType<lua_CFunction>(void) { return LUA_TFUNCTION; }
template<> constexpr inline int ThreadView::getType<lua_State*>(void) { return LUA_TTHREAD; }

template<typename T>
inline const char* ThreadView::toCString(T&& complexStr) {
#ifdef ELYSIAN_LUA_USE_STD_STRING
    if constexpr(std::is_same_v<std::decay<T>::type, std::string>)
        return complexStr.c_str();
    else
#endif
    return complexStr;
}

inline ThreadView::operator lua_State *(void)const { return m_pState; }

inline ThreadView::ThreadView(lua_State* pState): m_pState(pState) {}

inline int ThreadView::getTop(void) const { return lua_gettop(m_pState); }

inline lua_State *ThreadView::getState(void) const { return m_pState; }

inline bool ThreadView::isValid(void) const { return m_pState != nullptr; }
inline bool ThreadView::isValidIndex(int index) const {
    int absIndex = abs(index);
    return isValid() && absIndex >= 1 && absIndex >= getTop();
}

inline void ThreadView::_setState(lua_State *pState) { m_pState = pState; }

inline void ThreadView::push(void) const {}
inline void ThreadView::push(std::nullptr_t) const { pushNil(); }
inline void ThreadView::pushNil(void) const { lua_pushnil(m_pState); }
inline void ThreadView::push(lua_Integer integer) const { lua_pushinteger(m_pState, integer); }
inline void ThreadView::push(lua_Number number) const { lua_pushnumber(m_pState, number); }
inline void ThreadView::push(bool boolean) const { lua_pushboolean(m_pState, boolean); }
inline const char* ThreadView::push(const char* pString) const {
    const char* pRetStr = nullptr;
    if(pString) pRetStr = lua_pushstring(m_pState, pString);
    else pushNil();
    return pRetStr;
}
#ifdef ELYSIAN_LUA_USE_STD_STRING
inline const char* ThreadView::push(std::string cppStr) const {
    return lua_pushstring(m_pState, cppStr.c_str());
}
#endif
inline void ThreadView::push(void* pLightUd) const {
    if(pLightUd) lua_pushlightuserdata(m_pState, pLightUd);
    else lua_pushnil(m_pState);
}
inline void ThreadView::push(const lua_CFunction pCFunc) const {
    if(pCFunc) lua_pushcfunction(m_pState, pCFunc);
    else lua_pushnil(m_pState);
}
inline bool ThreadView::pushThread(void) const {
    return lua_pushthread(m_pState);
}
inline const char* ThreadView::pushStringVaList(const char *pFmt, va_list vaList) const {
  return lua_pushvfstring(m_pState, pFmt, vaList);
}
inline const char* ThreadView::pushStringBuffer(const char* pBuff, size_t length) const {
    return lua_pushlstring(m_pState, pBuff, length);
}
inline void ThreadView::pushGlobalsTable(void) const { lua_pushglobaltable(m_pState); }
inline void ThreadView::pushCClosure(const lua_CFunction fn, int upvalueCount) const {
    assert(upvalueCount < getTop());
    lua_pushcclosure(m_pState, fn, upvalueCount);
}
inline void ThreadView::pushValue(int index) const {
    assert(toAbsStackIndex(index) <= getTop());
    lua_pushvalue(m_pState, index);
}
template<typename T, size_t S>
inline void ThreadView::push(const std::array<T, S>& array) const {
    lua_createtable(m_pState, S, 0);
    for(size_t i = 0; i < S; ++i) {
        push(array[i]);
        lua_rawseti(m_pState, -2, static_cast<lua_Integer>(i + 1));
    }
}
inline bool ThreadView::checkStack(unsigned slots) const {
    return lua_checkstack(m_pState, static_cast<int>(slots));
}
inline int ThreadView::setCStackLimit(unsigned int limit) const {
    return lua_setcstacklimit(m_pState, limit);
}
inline void ThreadView::pop(int count) const {
    assert(count <= getTop());
    lua_pop(m_pState, count);
}

inline void ThreadView::remove(int index) const {
    lua_remove(m_pState, index);
}

inline void ThreadView::copy(int fromIndex, int toIndex) const {
    lua_copy(m_pState, fromIndex, toIndex);
}

inline void ThreadView::insert(int index) const {
    lua_insert(m_pState, index);
}

inline int ThreadView::getType(int index) const {
    assert(toAbsStackIndex(index) <= getTop());
    return lua_type(m_pState, index);
}
inline const char* ThreadView::getTypeName(int index) const {
    return lua_typename(m_pState, getType(index));
}
inline int ThreadView::toAbsStackIndex(int index) const {
    return lua_absindex(m_pState, index);
}
inline bool ThreadView::isInteger(int index) const {
    assert(toAbsStackIndex(index) <= getTop());
    return lua_isinteger(m_pState, index);
}

inline bool ThreadView::isBoolean(int index) const { return lua_isboolean(m_pState, index); }
inline bool ThreadView::isCFunction(int index) const { return lua_iscfunction(m_pState, index); }
inline bool ThreadView::isFunction(int index) const { return lua_isfunction(m_pState, index); }
inline bool ThreadView::isLightUserdata(int index) const { return lua_islightuserdata(m_pState, index); }
inline bool ThreadView::isNil(int index) const { return lua_isnil(m_pState, index); }
inline bool ThreadView::isNone(int index) const { return lua_isnone(m_pState, index); }
inline bool ThreadView::isNoneOrNil(int index) const { return lua_isnoneornil(m_pState, index); }
inline bool ThreadView::isNumber(int index) const { return lua_isnumber(m_pState, index); }
inline bool ThreadView::isString(int index) const { return lua_isstring(m_pState, index); }
inline bool ThreadView::isTable(int index) const { return lua_istable(m_pState, index); }
inline bool ThreadView::isThread(int index) const { return lua_isthread(m_pState, index); }
inline bool ThreadView::isUserdata(int index) const { return lua_isuserdata(m_pState, index); }

inline lua_Integer ThreadView::toInteger(int index, bool* pIsNum) const {
    int isNum;
    int retVal = lua_tointegerx(m_pState, index, &isNum);
    if(pIsNum) *pIsNum = static_cast<bool>(isNum);
    return retVal;
}

inline const char* ThreadView::toString(int index, size_t* pLen) const {
    return lua_tolstring(m_pState, index, pLen);
}

inline lua_Number ThreadView::toNumber(int index, bool* pIsNum) const {
    int isNum;
    lua_Number retVal = lua_tonumberx(m_pState, index, &isNum);
    if(pIsNum) *pIsNum = static_cast<bool>(isNum);
    return retVal;
}

inline bool ThreadView::toBoolean(int index) const {
    return lua_toboolean(m_pState, index);
}

inline lua_CFunction ThreadView::toCFunction(int index) const {
    return lua_tocfunction(m_pState, index);
}

inline void ThreadView::toClose(int index) const {
    return lua_toclose(m_pState, index);
}

inline const void* ThreadView::toPointer(int index) const {
    return lua_topointer(m_pState, index);
}

inline lua_State* ThreadView::toThread(int index) const {
    return lua_tothread(m_pState, index);
}

inline void* ThreadView::toUserdata(int index) const {
    return lua_touserdata(m_pState, index);
}

inline lua_Number ThreadView::getVersion(void) const {
    return lua_version(m_pState);
}

inline size_t ThreadView::stringToNumber(const char* pString) {
    return lua_stringtonumber(m_pState, pString);
}

template <typename T>
inline T ThreadView::toValue(int index) const { return T(); }

template<>
inline bool ThreadView::toValue<bool>(int index) const { return toBoolean(index); }

template<>
inline lua_Integer ThreadView::toValue<lua_Integer>(int index) const { return toInteger(index); }

template<>
inline lua_Number ThreadView::toValue<lua_Number>(int index) const { return toNumber(index); }

template<>
inline lua_CFunction ThreadView::toValue<lua_CFunction>(int index) const { return toCFunction(index); }

inline int ThreadView::getStatus(void) const { return lua_status(m_pState); }
inline bool ThreadView::isYieldable(void) const { return lua_isyieldable(m_pState); }

template<typename K>
inline int ThreadView::getGlobalsTable(K&& key) const {
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
inline int ThreadView::getGlobalsTable(K&& key, V& value) const {
    int retType = getGlobalsTable(key);
    pull(value);
    return retType;
}


template<typename K>
inline void ThreadView::setGlobalsTable(K&& key) {
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
inline void ThreadView::setGlobalsTable(K&& key, V&& value) {
    pushGlobalsTable();
    setTable(-1, key, value);
    pop(1);
}

template<typename First, typename... Rest>
inline void ThreadView::push(const LuaTableValues<First, Rest...>& tableValues) const {
    // THIS COULD POTENTIALLY DETERMINE THE CONTIGUOUS RANGE TOO AT COMPILE-TIME
    lua_createtable(m_pState, 0, std::tuple_size<std::remove_reference_t<decltype(tableValues)>>::value);
    appendTableRaw(-1, tableValues);
}

template<typename C, std::size_t... Is>
inline void ThreadView::appendSequence(int index, const C& container, std::index_sequence<Is...>) const {
    (setTable(index, std::get<Is>(container).first, std::get<Is>(container).second), ...);
}

template<typename C, std::size_t... Is>
inline void ThreadView::appendSequenceRaw(int index, const C& container, std::index_sequence<Is...>) const {
    (setTableRaw(index, std::get<Is>(container).first, std::get<Is>(container).second), ...);
}

template<typename First, typename... Rest>
inline void ThreadView::appendTable(int index, const LuaTableValues<First, Rest...>& tableValues) const {
    appendSequence(index, tableValues, std::index_sequence_for<First, Rest...>());
}

template<typename First, typename... Rest>
inline void ThreadView::appendTableRaw(int index, const LuaTableValues<First, Rest...>& tableValues) const {
    appendSequenceRaw(index, tableValues, std::index_sequence_for<First, Rest...>());
}

inline void ThreadView::setTable(int index) const {
    lua_settable(m_pState, index);
}

template<typename K, typename V>
inline void ThreadView::setTable(int index, const K& key, const V& value) const {
    internal::ThreadViewTableHelper<K>::set(this, index, key, value);
}

inline void ThreadView::setTableRaw(int index) const {
    lua_rawset(m_pState, index);
}

template<typename K, typename V>
inline void ThreadView::setTableRaw(int index, const K& key, const V& value) const {
    internal::ThreadViewTableHelper<K>::setRaw(this, index, key, value);
}

inline int ThreadView::getTable(int index) const {
    return lua_gettable(m_pState, index);
}

inline int ThreadView::getTableRaw(int index) const {
    return lua_rawget(m_pState, index);
}

template<typename T>
inline bool ThreadView::checkType(int index) const {
    return (getType<T>() == getType(index));
}

template<typename V>
inline bool ThreadView::pull(V& value) const {
    bool success = false;
    if(checkType<V>(-1)) {
        value = toValue<V>(-1);
        success = true;
        lua_pop(m_pState, 1);
    }
    return success;
}

template<typename K, typename V>
inline int ThreadView::getTable(int index, const K& key, V& value) const {
    const int retType = internal::ThreadViewTableHelper<K>::get(this, index, key);
    pull(value);
    return retType;
}

template<typename K>
inline int ThreadView::getTable(int index, K&& key) const {
    return internal::ThreadViewTableHelper<K>::get(this, index, key);
}

template<typename K, typename V>
inline int ThreadView::getTableRaw(int index, const K& key, V& value) const {
    const int retType = internal::ThreadViewTableHelper<K>::getRaw(this, index, key);
    pull(value);
    return retType;
}

template<typename K>
inline int ThreadView::getTableRaw(int index, K&& key) const {
    return internal::ThreadViewTableHelper<K>::getRaw(this, index, key);
}

template<typename... Keys>
inline int ThreadView::getTableMulti(int index, const std::tuple<Keys...>& keys) const {
    const int startTop = getTop();
    const int retVal = getTableMulti(index, keys, std::index_sequence_for<Keys...>());
    insert(startTop+1);
    pop(getTop()-(startTop+1));
    return retVal;
}

template<typename C, std::size_t... Is>
inline int ThreadView::getTableMulti(int index, const C& container, std::index_sequence<Is...>) const {
    pushValue(index);
    const int oldTop = getTop();
    int val = (getTable(-1, std::get<Is>(container)), ...);
    remove(oldTop);
    return val;
}

template<typename V, typename... Keys>
inline void ThreadView::setTableMulti(int index, const std::tuple<Keys...>& keys, V&& value) const {
    const int startTop = getTop();
    setTableMulti(index, keys, std::index_sequence_for<Keys...>(), value);
    insert(startTop+1);
    pop(getTop()-(startTop+1));
}

template<typename V, typename C, std::size_t... Is>
inline void ThreadView::setTableMulti(int index, const C& container, std::index_sequence<Is...>, V&& value) const {
    pushValue(index);
    const int oldTop = getTop();

    auto processElement = [&](const C& cont, auto Idx, V&& value) {
        if constexpr(Idx == std::index_sequence<Is...>::size()) {
            setTable(-1, std::get<Idx>(cont), value);
        } else {
            getTable(-1, std::get<Idx>(cont));
        }
    };

    (processElement(container, std::integral_constant<std::size_t, Is>(), std::forward<V>(value)), ...);
    pull(value);
    pop();
}

inline void ThreadView::close(void) const {
    lua_close(m_pState);
}


template<typename F>
void ThreadView::iteratePairs(int index, F&& body) const {

}

template<typename F>
void ThreadView::iterateiPairs(int index, F&& body) const {

}





}

#endif // ELYSIAN_LUA_THREAD_VIEW_HPP
