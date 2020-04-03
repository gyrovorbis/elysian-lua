#ifndef ELYSIAN_LUA_THREAD_HPP
#define ELYSIAN_LUA_THREAD_HPP

#include "elysian_lua_thread_view.hpp"

#define ELYSIAN_LUA_THREAD_NAME_SIZE 64

namespace elysian::lua {

// Actual thread
class Thread: public ThreadView {
  friend class LuaVM;

public:
  // call/callk
  // for pairs/ipairs loop

  // getMetaMethod
  // setMetaMethod
  //
  // all generic stack operations
  // coroutine shit

  // doFile
  // runString
  // load script

  static Thread *fromState(lua_State *pState);
  // Thread *fromStackRef(const LuaStackRef &ref);
  // getValue<thread> from LuaVariant?

  // coroutine resume and shit

  Thread(const char *pName);

private:
  static void *_luaAlloc(void *ud, void *ptr, size_t osize, size_t nsize);

  char _name[ELYSIAN_LUA_THREAD_NAME_SIZE];

  // std::vector<LuaStackFrame> _stackFrames;
  uint64_t _allocBytes = 0;
  uint64_t _allocCount = 0;
};

inline Thread *Thread::fromState(lua_State *pState) {
  void *pExtraSpace = lua_getextraspace(pState);
  assert(pExtraSpace);
  return *static_cast<Thread **>(pExtraSpace);
}

}

#endif // ELYSIAN_LUA_THREAD_HPP
