#ifndef ELYSIAN_LUA_STACK_HPP
#define ELYSIAN_LUA_STACK_HPP

#include <optional>
#include <map>
#include <vector>
#include "elysian_lua_thread_view_base.hpp"
#include "elysian_lua_traits.hpp"

namespace elysian::lua {

namespace stack_impl {

template<>
struct stack_checker<std::nullptr_t> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isNil(index);
    }
};

template<>
struct stack_pusher<std::nullptr_t> {
    static int push(const ThreadViewBase* pBase, StackRecord& record, std::nullptr_t) {
        pBase->pushNil();
        return 1;
    }
};

template<>
struct stack_getter<std::nullptr_t> {
    static std::nullptr_t get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return nullptr;
    }
};

template<typename T>
struct stack_checker<T, sfinae_decay_type<T, lua_Integer>> {
    static bool check(const ThreadViewBase* pBase, StackRecord&, int index) {
        return pBase->isInteger(index);
    }
};

template<typename T>
struct stack_getter<T, sfinae_decay_type<T, lua_Integer>> {
    static lua_Integer get(const ThreadViewBase* pBase, StackRecord&, int index) {
        return pBase->toInteger(index);
    }
};

template<typename T>
struct stack_pusher<T, sfinae_decay_type<T, lua_Integer>> {
    static int push(const ThreadViewBase* pBase, StackRecord&, lua_Integer value) {
        pBase->push(value);
        return 1;
    }
};


template<>
struct stack_checker<lua_Number> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isNumber(index);
    }
};

template<>
struct stack_getter<lua_Number> {
    static lua_Number get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->toNumber(index);
    }
};

template<>
struct stack_pusher<lua_Number> {
    static int push(const ThreadViewBase* pBase, StackRecord&, lua_Number value) {
        pBase->push(value);
        return 1;
    }
};


template<>
struct stack_checker<bool> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isBoolean(index);
    }
};

template<>
struct stack_getter<bool> {
    static bool get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->toBoolean(index);
    }
};

template<>
struct stack_pusher<bool> {
    static int push(const ThreadViewBase* pBase, StackRecord&, bool value) {
        pBase->push(value);
        return 1;
    }
};

template<>
struct stack_checker<const char*> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isString(index);
    }
};

template<>
struct stack_getter<const char*> {
    static const char* get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->toString(index);
    }
};

template<>
struct stack_pusher<const char*> {
    static int push(const ThreadViewBase* pBase, StackRecord&, const char* value) {
        pBase->push(value);
        return 1;
    }
};

template<>
struct stack_checker<void*> {
   static  bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isLightUserdata(index);
    }
};

template<>
struct stack_getter<void*> {
    static void* get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->toUserdata(index);
    }
};

template<>
struct stack_pusher<void*> {
    static int push(const ThreadViewBase* pBase, StackRecord&, void* value) {
        pBase->push(value);
        return 1;
    }
};

template<>
struct stack_checker<const void*> {
   static  bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isLightUserdata(index);
    }
};

template<>
struct stack_getter<const void*> {
    static const void* get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->toPointer(index);
    }
};

template<>
struct stack_pusher<const void*> {
    static int push(const ThreadViewBase* pBase, StackRecord&, const void* value) {
        pBase->push(value);
        return 1;
    }
};


template<>
struct stack_checker<lua_CFunction> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isCFunction(index);
    }
};

template<>
struct stack_getter<lua_CFunction> {
    static lua_CFunction get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->toCFunction(index);
    }
};

template<>
struct stack_pusher<lua_CFunction> {
    static int push(const ThreadViewBase* pBase, StackRecord&, lua_CFunction value) {
        pBase->push(value);
        return 1;
    }
};


template<>
struct stack_checker<lua_State*> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isThread(index);
    }
};

template<>
struct stack_getter<lua_State*> {
    static lua_State* get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->toThread(index);
    }
};

template<>
struct stack_pusher<lua_State*> {
    static int push(const ThreadViewBase* pBase, StackRecord& record, lua_State* pState) {
        assert(pBase->getState() == pState);
        pBase->pushThread();
        return 1;
    }
};

/* Not sure whether
 * a. Optional means it can either be nullptr or the correct type
 * b. Optional means it can be anything, and the wrong type is just
 *   treated as nullptr?
 *
 * a. is actually correct based on what optional does in C++?
 * b. seems very useful?
 */
template<typename T>
struct stack_checker<std::optional<T>> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return (pBase->isNil(index) || stack_checker<T>::check(pBase, record, index));
    }
};

template<typename T>
struct stack_getter<std::optional<T>> {
    static std::optional<T> get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        record.use(1);
        if(!pBase->isNil(index)) {
            return stack_getter<T>::get(pBase, record, index);
        } else {
            return {};
        }
    }
};

template<typename T>
struct stack_pusher<std::optional<T>> {
    static int push(const ThreadViewBase* pBase, StackRecord& record, const std::optional<T>& value) {
        record.use(1);
        if(!value.has_value()) {
            pBase->pushNil();
            return 1;
        } else {
            return stack_pusher<T>::push(pBase, record, value.value());
        }
    }
};

template<typename C>
struct index_sequence_stack_checker {
    template<std::size_t... Is>
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index, std::index_sequence<Is...>) {
        const int absIndex = pBase->toAbsStackIndex(index);
        return (stack_checker<std::tuple_element<Is, C>::type>::check(pBase, record, absIndex+Is) && ...);
    }

   static  bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return check(pBase, record, index, std::make_index_sequence<std::tuple_size<C>::value>{});
    }
};

template<typename C>
struct index_sequence_stack_getter {
    template<std::size_t... Is>
    static void fill(const ThreadViewBase* pBase, StackRecord& record, int index, C& cont, std::index_sequence<Is...>) {
        const int absIndex = pBase->toAbsStackIndex(index);
        ((std::get<Is>(cont) = stack_get<std::decay_t<decltype(std::get<Is>(cont))>>(pBase, record, index+Is)), ...);
    }

    static C get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        C tuple;
        fill(pBase, record, index, tuple, std::make_index_sequence<std::tuple_size<C>::value>{});
        return tuple;
    }
};

template<typename C>
struct index_sequence_stack_pusher {
    template<std::size_t... Is>
    static int pushSequence(const ThreadViewBase* pBase, StackRecord& record, const C& cont, std::index_sequence<Is...>) {
        return (stack_pusher<std::tuple_element<Is, C>::type>::push(pBase, record, std::get<Is>(cont)) + ...);
    }

    static int push(const ThreadViewBase* pBase, StackRecord& record, const C& cont) {
        return pushSequence(pBase, record, cont, std::make_index_sequence<std::tuple_size<C>::value>{});
    }
};

template<typename... Args>
constexpr const static int stack_count<std::tuple<Args...>> = (stack_count<Args> + ...);

template<typename... Args>
struct stack_checker<std::tuple<Args...>>:
    public index_sequence_stack_checker<std::tuple<Args...>>
{};

/*
template<typename... Args>
struct stack_getter<std::tuple<Args&...>>:
    public index_sequence_stack_getter<std::tuple<Args&...>>
{
    using ValueTuple = std::tuple<std::decay_t<Args>...>;

    static std::tuple<Args...> get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        ValueTuple tuple;
        fill(pBase, record, index, tuple, std::make_index_sequence<std::tuple_size<ValueTuple>::value>{});
        return tuple;
    }

};*/

template<typename... Args>
struct stack_getter<std::tuple<Args...>>:
    public index_sequence_stack_getter<std::tuple<std::decay_t<Args>...>>
{};

template<typename... Args>
struct stack_pusher<std::tuple<Args...>>:
    public index_sequence_stack_pusher<std::tuple<Args...>>
{};

template<typename T, size_t S>
constexpr const static bool stack_table_type<std::array<T, S>> = true;

template<typename T, size_t S>
struct stack_pusher<std::array<T, S>> {
    static int push(const ThreadViewBase* pBase, StackRecord& record, const std::array<T, S>& array) {
        pBase->pushNewTable(S);
        for(size_t i = 0; i < S; ++i) {
            pBase->setTableRaw(-1, i+1, array[i]);
        }
        return 1;
    }
};

template<typename T, size_t S>
constexpr const static bool stack_table_type<T[S]> = true;

template<typename T, size_t S>
struct stack_pusher<T[S]> {
    static int push(const ThreadViewBase* pBase, StackRecord& record, const T (&array)[S]) {
        pBase->pushNewTable(S);
        for(size_t i = 0; i < S; ++i) {
            pBase->setTableRaw(-1, i+1, array[i]);
        }
        return 1;
    }
};

template<typename... Args>
constexpr const static bool stack_table_type<LuaTableValues<Args...>> = true;

template<typename... Args>
struct stack_pusher<LuaTableValues<Args...>> {
    static int push(const ThreadViewBase* pBase, StackRecord& record, const LuaTableValues<Args...>& tableValues) {
        // THIS COULD POTENTIALLY DETERMINE THE CONTIGUOUS RANGE TOO AT COMPILE-TIME
        pBase->pushNewTable(0, std::tuple_size<std::tuple<Args...>>::value);
        pBase->appendTableRaw(-1, tableValues);
        return 1;
    }
};

template<typename Key, typename Value>
constexpr const static bool stack_table_type<std::map<Key, Value>> = true;

template<typename Key, typename Value>
struct stack_checker<std::map<Key, Value>> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        bool valid = pBase->isTable(index);
        if(valid) {
            valid = pBase->iterateTable(index, [&](){
                if(!stack_check<Key>(pBase, record, -2)) {
                    return false;
                }
                if(!stack_check<Value>(pBase, record, -1)) {
                    return false;
                }
                return true;
            });
        }
        return valid;
     }
};

template<typename Key, typename Value>
struct stack_getter<std::map<Key, Value>> {
    static std::map<Key, Value> get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        std::map<Key, Value> tableMap;
        pBase->iterateTable(index, [&](){
            tableMap.emplace(pBase->toValue<Key>(-2), pBase->toValue<Value>(-1));
        });
        return tableMap;
     }
};

template<typename Key, typename Value>
struct stack_pusher<std::map<Key, Value>> {
    static bool push(const ThreadViewBase* pBase, StackRecord& record, const std::map<Key, Value>& tableMap) {
        if constexpr(std::is_integral_v<Key>)
            pBase->pushNewTable(tableMap.size());
        else
            pBase->pushNewTable(0, tableMap.size());

        for(auto it = tableMap.begin(); it != tableMap.end(); ++it) {
            pBase->setTableRaw(-1, it->first, it->second);
        }
        return true;
    }
};





}








}

#endif // ELYSIAN_LUA_STACK_HPP
