#include <ElysianLua/elysian_lua_thread.hpp>
#include <cstring>

namespace elysian::lua {



Thread::Thread(const char *pName) {
  assert(pName);
  strncpy(_name, pName, ELYSIAN_LUA_THREAD_NAME_SIZE);
}



}
