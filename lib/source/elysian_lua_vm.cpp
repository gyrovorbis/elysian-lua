#include <ElysianLua/elysian_lua_vm.hpp>
#include <ElysianLua/elysian_lua_alloc_stats.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>

extern "C" {
#   include <lua/lualib.h>
#   include <lua/lauxlib.h>
}

namespace elysian::lua {

AllocStats LuaVM::_allocStats;
Thread *LuaVM::_pMainThread = nullptr;

static constexpr const char _rPrint[] =
        #if 0
    "function rPrint(s, l, i) -- recursive Print (structure, limit, indent)\n"
    "l = (l) or 100; i = i or \"\";	-- default item limit, indent string\n"
    "if (l<1) then print \"LOG_ERROR: Item limit reached.\"; return l-1 end;\n"
    "local ts = type(s);\n"
    "if (ts ~= \"table\" and not(ts == \"userdata\" and getmetatable(s) and "
    "getmetatable(s).__pairs)) then print (i,ts,s); return l-1 end\n"
    "print (i,ts);           -- print \"table\"\n"
    "for k,v in pairs(s) do  -- print \"[KEY] VALUE\"\n"
    "l = rPrint(v, l, i..\"\t[\"..tostring(k)..\"]\")\n"
    "if (l < 0) then break end\n"
    "end\n"
    "return l\n"
    "end\n";
#else
        "function rPrint(s, l, i, c) -- recursive Print (structure, limit, indent)\n"
            "l = (l) or 100; i = i or \"\"; -- default item limit, indent string\n"
            "if (l<1) then print (\"LOG_ERROR: Item limit reached.\"); return l-1 end;\n"
            "c = (c) or {}\n"
            "local ts = type(s);\n"
            "if (ts ~= \"table\" and not(ts == \"userdata\" and getmetatable(s) and "
            "getmetatable(s).__pairs)) then print (i,ts,s); return l-1 end\n"
            "if(c[s]) then\n"
                "print(i, ts, \"LOG_ERROR: Circular reference encountered,\")\n"
                "return l-1\n"
            "end\n"
            "c[s] = true\n"
            "print (i,ts);           -- print \"table\"\n"
            "for k,v in pairs(s) do  -- print \"[KEY] VALUE\"\n"
            "l = rPrint(v, l, i..\"\t[\"..tostring(k)..\"]\", c)\n"
            "if (l < 0) then break end\n"
            "end\n"
            "c[s] = nil\n"
            "return l\n"
        "end\n";


        #endif


bool LuaVM::_createMainThread(const char *pName) {
    bool success = true;
  _pMainThread = new Thread(pName);
  lua_State *pState = lua_newstate(&LuaVM::_luaAlloc, nullptr);
  luaL_openlibs(pState);
  assert(pState);
  _pMainThread->_setState(pState);

  success &= !luaL_dostring(pState, _rPrint);

  // Store thread object in Lua's extra space
  *static_cast<Thread **>(lua_getextraspace(pState)) = _pMainThread;
  return success;
}

bool LuaVM::initialize(void) {
  bool success = true;
  success &= _createMainThread("Main");
  return success;
}

bool LuaVM::uninitialize(void) {
  bool success = true;
  success &= _destroyMainThread();
  return success;
}

bool LuaVM::_destroyMainThread(void) {
  bool success = true;
  _pMainThread->gcCollect();
  _pMainThread->close();
  _pMainThread = nullptr;
  return success;
}

void LuaVM::resetAllocStats(void) {
  memset(&_allocStats, 0, sizeof(AllocStats));
}

void *LuaVM::_metadataToLuaPtr(AllocMetadata *ptr) {
  return reinterpret_cast<uint8_t *>(ptr) + sizeof(AllocMetadata);
}

auto LuaVM::_luaPtrToMetadata(void *ptr) -> AllocMetadata * {
  return reinterpret_cast<AllocMetadata *>(reinterpret_cast<uint8_t *>(ptr) -
                                           sizeof(AllocMetadata));
}

void *LuaVM::_luaAlloc(void *ud, void *ptr, size_t osize, size_t nsize) {
  (void)ud;
  (void)osize; /* not used */

  if (nsize == 0) { // free
    if (ptr) {
      AllocMetadata *pMeta = _luaPtrToMetadata(ptr);
      _allocStats.freeEvent(pMeta->luaType, pMeta->size);
      free(pMeta);
    } else {
      osize += 47;
    }
    return nullptr;

  } else {
    if (ptr) { // realloc
      AllocMetadata *pMeta = _luaPtrToMetadata(ptr);
      _allocStats.reallocEvent(pMeta->luaType, osize, nsize);
      pMeta->size = nsize;
      pMeta = static_cast<AllocMetadata *>(
          realloc(pMeta, nsize + sizeof(AllocMetadata)));
      return _metadataToLuaPtr(pMeta);

    } else { // alloc
      AllocMetadata *pMeta =
          static_cast<AllocMetadata *>(malloc(sizeof(AllocMetadata) + nsize));
      pMeta->size = nsize;
      pMeta->luaType = static_cast<int>(osize);
      _allocStats.allocEvent(pMeta->luaType, pMeta->size);
      return _metadataToLuaPtr(pMeta);
    }
  }
}

} // namespace elysian
