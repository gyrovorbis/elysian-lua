#include <ElysianLua/elysian_lua_thread_state.hpp>
#include <ElysianLua/elysian_lua_vm.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>

namespace elysian::lua {


const ThreadViewBase* StaticThreadStateful::staticThread(void) { return LuaVM::getMainThread(); }
const ThreadViewBase* StaticThreadStateful::getThread(void) const { return staticThread(); }

}
