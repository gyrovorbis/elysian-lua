#ifndef ELYSIAN_LUA_THREAD_VIEW_HPP
#define ELYSIAN_LUA_THREAD_VIEW_HPP

#include "elysian_lua_thread_view_base.hpp"
#include "elysian_lua_stack.hpp"
#include "elysian_lua_proxy.hpp"
#include "elysian_lua_table.hpp"
#include "elysian_lua_function_result.hpp"
#include "elysian_lua_function.hpp"

namespace elysian::lua {

class ThreadView: public ThreadViewBase {
public:
    ThreadView(lua_State* state);

    auto script(const char* pString, Table envTable=Table()) const -> ProtectedFunctionResult;
    auto scriptFile(const char* pFilePath, Table envTable=Table()) const -> ProtectedFunctionResult;

    using ThreadViewBase::createTable;

    template<typename T>
    std::enable_if_t<stack_impl::stack_table_type<T>, StackTable>
    createTable(const T& table) const;

    template<typename K>
    GlobalsTableProxy<K> operator[](K key);

protected:
    ThreadView(void) = default;

};

inline ThreadView::ThreadView(lua_State* state):
    ThreadViewBase(state) {}

template<typename T>
inline std::enable_if_t<stack_impl::stack_table_type<T>, StackTable>
ThreadView::createTable(const T& value) const {
    push(value);
    return toValue<StackTable>(-1);
}

template<typename K>
inline GlobalsTableProxy<K> ThreadView::operator[](K key) {
    return GlobalsTableProxy<K>(getGlobalsTable(), std::make_tuple(std::move(key)));
}

}

#endif // ELYSIAN_LUA_THREAD_VIEW_HPP
