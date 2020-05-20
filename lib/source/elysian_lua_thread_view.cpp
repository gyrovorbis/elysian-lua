#include <ElysianLua/elysian_lua_thread.hpp>
#include <ElysianLua/elysian_lua_object.hpp>
#include <ElysianLua/elysian_lua_reference.hpp>
#include <ElysianLua/elysian_lua_function_result.hpp>
#include <cstdarg>

namespace elysian::lua {

const char* ThreadViewBase::pushStringFormatted(const char *pFmt, ...) const {
    va_list args;

    va_start(args, pFmt);
    const char* pRetStr = pushStringVaList(pFmt, args);
    va_end(args);

    return pRetStr;
}

void ThreadViewBase::setCurrentCppExecutionContext(const CppExecutionContext& ctx) const {
    Thread* pThread = Thread::fromState(*this);
    if(pThread) {
        pThread->setCurrentCppExecutionContext(ctx);
    }
}

void ThreadViewBase::syncCppCallerContexts(void) const {
    Thread* pThread = Thread::fromState(*this);
    if(pThread) {
        pThread->syncCppCallerContexts();
    }
}

void ThreadViewBase::rPrint(int index, unsigned maxDepth, const char* pLabel) {
    const int absIndex = toAbsStackIndex(index);
    const int retType = getGlobalsTable("rPrint");
    assert(retType == LUA_TFUNCTION);
    pushValue(absIndex);
    push(static_cast<lua_Integer>(maxDepth));
    push(pLabel);
    call(3, 0);
}

void ThreadViewBase::rPrintCStack(int startIndex, int endIndex, unsigned maxDepth, const char* pLabel) {
    char labelBuff[256]; //40000 max size, so max of 5 digits if we numbered everything

    if(getTop()) {

        if(!endIndex) {
            endIndex = getTop() * ((startIndex > 0)? 1 : -1);
        }

        assert((startIndex > 0 && endIndex > 0)
               || (startIndex < 0 && endIndex < 0));

        const int incr = (endIndex < startIndex)? -1 : 1;
        endIndex += incr;

        for(int i = startIndex; i != endIndex; i += incr) {
            snprintf(labelBuff, sizeof(labelBuff), "%s[%d]", pLabel, i);
            rPrint(i, maxDepth, labelBuff);
        }
    }
}

GlobalsTable ThreadViewBase::getGlobalsTable(void) {
    return GlobalsTable(this);
}

StackTable ThreadViewBase::createTable(int arraySize, int hashSize) const {
    pushNewTable(arraySize, hashSize);
    return toValue<StackTable>(-1);
}

auto ThreadView::script(const char* pString, Table envTable)
    const -> ProtectedFunctionResult
{
    const int oldTop = getTop();
    int errorCode = loadString(pString);
    if(errorCode == LUA_OK) {
        if(envTable) {
            if(push(envTable)) {
                setUpValue(-2, 1);
            }
        }
        errorCode = pCall(0, LUA_MULTRET);
    }
    return ProtectedFunctionResult(this,
                                   errorCode,
                                   oldTop + 1,
                                   getTop() - oldTop);
}

auto ThreadView::scriptFile(const char* pFileName, Table envTable)
    const -> ProtectedFunctionResult
{
    const int oldTop = getTop();
    int errorCode = loadFile(pFileName);
    if(errorCode == LUA_OK) {
        if(envTable) {
            if(push(envTable)) {
                setUpValue(-2, 1);
            }
        }
        errorCode = pCall(0, LUA_MULTRET);
    }
    return ProtectedFunctionResult(this,
                                   errorCode,
                                   oldTop + 1,
                                   getTop() - oldTop);
}

}
