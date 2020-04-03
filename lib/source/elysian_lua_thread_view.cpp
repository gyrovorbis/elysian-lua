#include <ElysianLua/elysian_lua_thread_view.hpp>
#include <ElysianLua/elysian_lua_object.hpp>
#include <ElysianLua/elysian_lua_reference.hpp>
#include <cstdarg>

namespace elysian::lua {

const char* ThreadView::pushStringFormatted(const char *pFmt, ...) const {
    va_list args;

    va_start(args, pFmt);
    const char* pRetStr = pushStringVaList(pFmt, args);
    va_end(args);

    return pRetStr;
}

void ThreadView::rPrint(int index, unsigned maxDepth, const char* pName) {
    const int absIndex = toAbsStackIndex(index);
    const int retType = getGlobalsTable("rPrint");
    assert(retType == LUA_TFUNCTION);
    pushValue(absIndex);
    push(static_cast<lua_Integer>(maxDepth));
    push(pName);

    lua_call(m_pState, 3, 0);
}

GlobalsTable ThreadView::getGlobalsTable(void) {
    return GlobalsTable(this);
}

}
