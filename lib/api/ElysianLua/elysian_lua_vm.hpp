#ifndef ELYSIAN_LUA_VM_HPP
#define ELYSIAN_LUA_VM_HPP

#include <vector>

extern "C" {
    #include <lua/lua.h>
}

namespace elysian::lua {

class AllocStats;
class Thread;

class LuaVM {
public:
  static_assert(LUA_EXTRASPACE >= sizeof(Thread *),
                "Not enough space to store thread pointers in lua_State!");

  static bool initialize(void);
  static bool uninitialize(void);

  static Thread *getMainThread(void);
  static Thread *getThread(const char *pName);
  static Thread *getThread(unsigned index);
  static Thread *createThread(const char *pName);
  static Thread *removeThread(const char *pName);
  static unsigned getThreadCount(void);

  static const AllocStats &getAllocStats(void);
  static void resetAllocStats(void);

  // lua_register - c function
  // lua_setallocf
  // lua_setWarnf
  // lua_version
  // lua_atpanic

  static void warn(const char *pString, ...);
  static void warn(const char *pString, va_list varArgs);

private:
  struct AllocMetadata {
    int luaType;
    size_t size;
  };

  static bool _createMainThread(const char *pName);
  static bool _destroyMainThread(void);

  static void *_luaAlloc(void *ud, void *ptr, size_t osize, size_t nsize);
  static AllocMetadata *_luaPtrToMetadata(void *ptr);
  static void *_metadataToLuaPtr(AllocMetadata *ptr);

  static Thread *_pMainThread;
  static std::vector<Thread *> _threads;
  static AllocStats _allocStats;
};

inline const AllocStats &LuaVM::getAllocStats(void) { return _allocStats; }
inline Thread *LuaVM::getMainThread(void) { return _pMainThread; }


} // namespace elysian

#endif // ELYSIAN_LUA_VM_HPP
