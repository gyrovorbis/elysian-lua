#include <cassert>

#include <ElysianLua/elysian_lua_table.hpp>
#include <ElysianLua/elysian_lua_stack_monitor.hpp>
#include <ElysianLua/elysian_lua_vm.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>

namespace elysian::lua {

 StackMonitor::StackMonitor(const ThreadViewBase* pThread):
    Monitor<StackMonitor, int> (pThread->getTop()),
    m_pThread(pThread)
{}

int StackMonitor::getCurrentValue(void) const { assert(getThread()); return getThread()->getTop(); }

int64_t RegistryRefCountMonitor::getCurrentValue(void) const { return LuaVM::getRegistryRefCount(); }

}
