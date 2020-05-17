#ifndef ELYSIAN_LUA_TUPLE_HPP
#define ELYSIAN_LUA_TUPLE_HPP

#include <tuple>

#include "elysian_lua_stack.hpp"
#include "elysian_lua_thread_base.hpp"


struct StackRecord {
    int totalUsed = 0; // Total number of items consumed/produced
    int lastUsed = 0; // Number of items consumed/produced by last operation

    void use(int count);
};

inline void StackRecord::use(int count) {
    lastUsed = count;
    totalUsed += count;
}

template<typename T, typename=void>
struct stack_checker;

template<typename T, typename=void>
struct stack_pusher;

template<typename T, typename=void>
struct stack_getter;

namespace elysian::lua::stack {


template<typename... Args>
struct stack_checker<std::tuple<Args...>> {

    static bool check(ThreadBase* pThread, StackRecord& record) {
        return (pThread->checkType<Args>(-std::index_sequence_for<Args>()) && ...);
    }
};

template<typename... Args>
struct stack_pusher<std::tuple<Args...>> {

    static bool push(ThreadBase* pThread, StackRecord& record, const std::tuple<Args...>& tuple) {

    }
};


template<typename... Args>
struct stack_getter<std::tuple<Args...>> {

    static bool get(ThreadBase* pThread, StackRecord& record) {
        return (pThread->checkType<Args>(-std::index_sequence_for<Args>()) && ...);
    }
};








}

#endif // ELYSIAN_LUA_TUPLE_HPP
