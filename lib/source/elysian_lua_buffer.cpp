#include <ElysianLua/elysian_lua_buffer.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>

namespace elysian::lua {

ThreadView Buffer::getThread(void) {
    return ThreadView(L);
}

}
