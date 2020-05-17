#include <ElysianLua/elysian_lua_reference.hpp>
#include <ElysianLua/elysian_lua_thread.hpp>
#include <ElysianLua/elysian_lua_vm.hpp>

extern "C" {
    #include <lua/lauxlib.h>
}

namespace elysian::lua {

const ThreadViewBase* StaticRefState::getThread(void) const { return LuaVM::getMainThread(); }

bool StatelessGlobalsTablePsuedoReference::push(const ThreadViewBase* pThread) const {
    if(pThread) {
        pThread->pushGlobalsTable();
        return true;
    } else {
        return false;
    }
}

bool StatelessGlobalsTablePsuedoReference::destroy(const ThreadViewBase* pThread) {
    return true;
}

bool StatelessRegistryReference::isValid(const ThreadViewBase* pThread) const { return m_ref != LUA_REFNIL && m_ref != LUA_NOREF && pThread && pThread->isValid(); }


bool StatelessRegistryReference::copy(const ThreadViewBase* pThread, const StatelessRegistryReference& rhs) {
    bool success = false;
    if(pThread && rhs.push(pThread)) {
        success = pull(pThread);
    } else {
        m_ref = LUA_NOREF;
    }
    return success;
}

bool StatelessRegistryReference::copy(const ThreadViewBase*, StatelessRegistryReference&& rhs) {
    m_ref = rhs.m_ref;
    rhs.m_ref = LUA_NOREF;
    return true;
}

bool StatelessRegistryReference::push(const ThreadViewBase* pThread) const {
    bool success = false;
    if(pThread) {
        if(m_ref == LUA_NOREF) {
            pThread->pushNil();
            success = true;
        } else {
            int retVal = pThread->getTableRaw(LUA_REGISTRYINDEX, m_ref);
            if(retVal != LUA_TNONE) {
                success = true;
            }
        }
    }
    return success;
}

bool StatelessRegistryReference::fromStackIndex(const ThreadViewBase* pThread, int index) {
    bool success = false;
    if(pThread) {
        pThread->pushValue(index);
        success = pull(pThread);
    }
    return success;
}

bool StatelessRegistryReference::pull(const ThreadViewBase* pThread) {
    bool success = false;
    if(pThread) {
        if(pThread->isObject(-1)) {
            //destroy(pThread);
            m_ref = pThread->ref();
            success = true;
        }
    }
    return success;
}

bool StatelessRegistryReference::destroy(const ThreadViewBase* pThread) {
    bool result = false;
    if(pThread) {
        pThread->unref(m_ref);
        m_ref = LUA_NOREF;
        result = true;
    }
    return result;
}




}
