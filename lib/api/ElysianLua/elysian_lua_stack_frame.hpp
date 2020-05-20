#ifndef ELYSIAN_LUA_STACK_FRAME_HPP
#define ELYSIAN_LUA_STACK_FRAME_HPP

namespace elysian::lua {

class CppExecutionContext {
public:
    CppExecutionContext(const char* pFile=nullptr, const char* pFunc=nullptr, int line=-1, const char* pType=nullptr);

    operator bool() const;

    bool isEmpty(void) const;

    bool hasFileName(void) const;
    bool hasFunctionName(void) const;
    bool hasLineNumber(void) const;
    bool hasTypeName(void) const;

    const char* getFileName(void) const;
    const char* getFunctionName(void) const;
    const char* getTypeName(void) const;
    int getLineNumber(void) const;

    void setFileName(const char* pName);
    void setFunctionName(const char* pName);
    void setTypeName(const char* pType);
    void setLineNumber(int line);

    void clear(void);

private:
    const char* m_pFile = nullptr;
    const char* m_pFunc = nullptr;
    const char* m_pType = nullptr;
    int m_line = -1;
    bool m_protected = false;
    bool m_coroutine = false;
};



inline CppExecutionContext::CppExecutionContext(const char* pFile, const char* pFunc, int line, const char* pType):
    m_pFile(pFile),
    m_pFunc(pFunc),
    m_pType(pType),
    m_line(line)
{}
inline CppExecutionContext::operator bool() const { return !isEmpty(); }
inline const char* CppExecutionContext::getFileName(void) const { return m_pFile; }
inline const char* CppExecutionContext::getFunctionName(void) const { return m_pFunc; }
inline const char* CppExecutionContext::getTypeName(void) const { return m_pType; }
inline int CppExecutionContext::getLineNumber(void) const { return m_line; }
inline bool CppExecutionContext::hasFileName(void) const { return m_pFile; }
inline bool CppExecutionContext::hasFunctionName(void) const { return m_pFunc; }
inline bool CppExecutionContext::hasTypeName(void) const { return m_pType; }
inline bool CppExecutionContext::hasLineNumber(void) const { return m_line != -1; }
inline bool CppExecutionContext::isEmpty(void) const {
    return !hasFileName() && !hasFunctionName() && !hasLineNumber() && !hasTypeName();
}
inline void CppExecutionContext::setFileName(const char* pName) { m_pFile = pName; }
inline void CppExecutionContext::setFunctionName(const char* pName) { m_pFunc = pName; }
inline void CppExecutionContext::setTypeName(const char* pName) { m_pType = pName; }
inline void CppExecutionContext::setLineNumber(int line) { m_line = line; }
inline void CppExecutionContext::clear(void) {
    setFileName(nullptr); setFunctionName(nullptr); setLineNumber(-1); setTypeName(nullptr);
}



}


#endif // ELYSIAN_LUA_STACK_FRAME_HPP
