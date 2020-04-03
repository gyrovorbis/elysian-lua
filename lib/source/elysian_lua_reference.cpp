#include <ElysianLua/elysian_lua_reference.hpp>
#include <ElysianLua/elysian_lua_thread_view.hpp>

namespace elysian::lua {

bool StatelessStackReference::isValid(const ThreadView* pThread) const { return m_index != 0 && pThread && pThread->isValidIndex(m_index); }

bool StatelessRegistryReference::isValid(const ThreadView* pThread) const { return m_pRef != LUA_REFNIL && m_pRef != LUA_NOREF && pThread && pThread->isValid(); }

}
