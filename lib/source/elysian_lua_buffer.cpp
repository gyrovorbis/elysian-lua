#include <ElysianLua/elysian_lua_buffer.hpp>
#include <ElysianLua/elysian_lua_thread_view.hpp>

namespace elysian::lua {

ThreadView Buffer::getThread(void) {
    return ThreadView(L);
}

}
