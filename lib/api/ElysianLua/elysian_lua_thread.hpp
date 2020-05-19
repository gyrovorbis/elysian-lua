#ifndef ELYSIAN_LUA_THREAD_HPP
#define ELYSIAN_LUA_THREAD_HPP

#include "elysian_lua_thread_view.hpp"
#include "elysian_lua_stack_frame.hpp"

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
  // all generic stack operationshow
  // coroutine shit

  // doFile
  // runString
  // load script

  static Thread *fromState(lua_State *pState);
  // Thread *fromStackRef(const LuaStackRef &ref);
  // getValue<thread> from LuaVariant?

  // coroutine resume and shit

  Thread(const char *pName);

  void setCurrentCppExecutionContext(const CppExecutionContext& ctx) const;
  void syncCppCallerContexts(void) const;
  const CppExecutionContext* getCppCallerContext(int recordLevel) const;

private:
    static void *_luaAlloc(void *ud, void *ptr, size_t osize, size_t nsize);

    mutable std::vector<CppExecutionContext> m_cppCallerContexts;

    char    _name[ELYSIAN_LUA_THREAD_NAME_SIZE];
    Thread*  m_parentState = nullptr;

    // std::vector<LuaStackFrame> _stackFrames;
    uint64_t _allocBytes = 0;
    uint64_t _allocCount = 0;
};

inline Thread *Thread::fromState(lua_State *pState) {
  void *pExtraSpace = lua_getextraspace(pState);
  assert(pExtraSpace);
  return *static_cast<Thread **>(pExtraSpace);
}

inline void Thread::setCurrentCppExecutionContext(const CppExecutionContext& ctx) const {
    const int depth = getFunctionCallDepth();
    assert(depth >= m_cppCallerContexts.size());

    const int growCount = depth - m_cppCallerContexts.size();
    if(growCount) m_cppCallerContexts.resize(depth, CppExecutionContext());
    m_cppCallerContexts.push_back(ctx);
}

inline void Thread::syncCppCallerContexts(void) const {
    const int depth = getFunctionCallDepth();
    assert(m_cppCallerContexts.size());
    assert(depth < m_cppCallerContexts.size());
    m_cppCallerContexts.resize(depth, CppExecutionContext());
}

inline const CppExecutionContext* Thread::getCppCallerContext(int recordLevel) const {
    return (recordLevel < m_cppCallerContexts.size())? &m_cppCallerContexts[recordLevel] : nullptr;
}

}

#endif // ELYSIAN_LUA_THREAD_HPP
