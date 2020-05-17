#ifndef ELYSIAN_LUA_THREAD_VIEW_HPP
#define ELYSIAN_LUA_THREAD_VIEW_HPP

#include "elysian_lua_thread_view_base.hpp"
#include "elysian_lua_stack.hpp"
#include "elysian_lua_proxy.hpp"
#include "elysian_lua_table.hpp"

namespace elysian::lua {

class ThreadView: public ThreadViewBase {
public:
    ThreadView(lua_State* state);

    auto script(const char* pString, Table envTable=Table()) const -> ProtectedFunctionResult;
    auto scriptFile(const char* pFilePath, Table envTable=Table()) const -> ProtectedFunctionResult;

protected:
    ThreadView(void) = default;

};

inline ThreadView::ThreadView(lua_State* state):
    ThreadViewBase(state) {}

}

#endif // ELYSIAN_LUA_THREAD_VIEW_HPP
