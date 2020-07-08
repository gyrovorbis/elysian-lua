
#include <ElysianLua/elysian_lua_reference.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>
#include <ElysianLua/elysian_lua_vm.hpp>

extern "C" {
    #include <lua/lauxlib.h>
}

namespace elysian::lua {

const ThreadViewBase* StaticRefState::getThread(void) const { return staticThread(); }
const ThreadViewBase* StaticRefState::staticThread(void) { return LuaVM::getMainThread(); }


}
