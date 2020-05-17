#ifndef ELYSIAN_LUA_BUFFER_HPP
#define ELYSIAN_LUA_BUFFER_HPP

#include <cstring>
#include <cassert>

extern "C" {
#   include <lua/lua.h>
#   include <lua/lualib.h>
#   include <lua/lauxlib.h>
}

namespace elysian::lua {

#if 0
Buffer blah;
thread().bufferInit(&blah);



blah.addFormatted("%1 %2 %3", va1, val2, val3);

blah.in

blah += Buffer::Fmt{"%1, %2, %3", val1, val2, val3}
        Buffer::Sprintf()


        thread().createBuffer()
        #endif



    class ThreadView;

class Buffer: public luaL_Buffer {
public:
    Buffer(void);

    // === Raw Lua Buffer API ===
    char* prep(size_t size=LUAL_BUFFERSIZE);
    char* getAddress(void);
    size_t getLength(void);
    void sub(int n);
    void pushResult(void);
    void pushResultSize(size_t size);
    void addChar(char c);
    void addGSub(const char* pString, const char* pRemoveSubStr, const char* pReplacementSubStr);
    void addLString(const char* pString, size_t length);
    void addString(const char* pString);
    void addSize(size_t n);
    void addValue(void);

    // === Actually useful API ===
    bool isValid(void);
    bool isEmpty(void);

    size_t pos(void);
    void posReset(void);
    void posMove(int delta);

    //void size(void);

    void reserveBytes(size_t n);

    template<typename... Args>
    Buffer& sprintf(const char* pFormatter, Args&&... args);

    operator const char*();
    const Buffer& operator=(const char* pString);
    const Buffer& operator+=(const char* pString);
    template<typename T>
    const Buffer& operator+=(const T& rhs);
    bool operator==(const char* pStr);
    bool operator!=(const char* pStr);

    template<typename T>
    Buffer& operator <<(T&& value);

protected:
    ThreadView getThread(void);



#ifdef ELYSIAN_LUA_USE_STD_STRING
    const Buffer& operator=(const std::string& string);
    const Buffer& operator+=(const std::string& string);
    bool operator==(const std::string& rhs);
    bool operator!=(const std::string& rhs);
#endif
};




    inline Buffer::Buffer(void) {
        memset(this, 0, sizeof(Buffer));
    }
    inline char* Buffer::prep(size_t size) { return luaL_prepbuffsize(this, size); }
    inline char* Buffer::getAddress(void) { return luaL_buffaddr(this); }
    inline size_t Buffer::getLength(void) { return luaL_bufflen(this); }
    inline void Buffer::sub(int n) { luaL_buffsub(this, n); }
    inline void Buffer::pushResult(void) { luaL_pushresult(this); }
    inline void Buffer::pushResultSize(size_t size) { luaL_pushresultsize(this, size); }
    inline void Buffer::addChar(char c) { luaL_addchar(this, c); }
    inline void Buffer::addGSub(const char* pString, const char* pRemoveSubStr, const char* pReplacementSubStr) {
        luaL_addgsub(this, pString, pRemoveSubStr, pReplacementSubStr);
    }
    inline void Buffer::addLString(const char* pString, size_t length) { luaL_addlstring(this, pString, length); }
    inline void Buffer::addString(const char* pString) { luaL_addstring(this, pString); }
    inline void Buffer::addSize(size_t n) { luaL_addsize(this, n); }
    inline void Buffer::addValue(void) { luaL_addvalue(this); }

    inline bool Buffer::isValid(void) { return getAddress() != nullptr; }
    inline bool Buffer::isEmpty(void) { return getLength() == 0; }

    inline Buffer::operator const char*() { return getAddress(); }

    inline const Buffer& Buffer::operator=(const char* pString) {
        assert(isValid() && pString);
        auto newLen = std::strlen(pString);
        sub(getLength()); //revert cursor position back to beginning
        char* pPos = prep(newLen);
        std::strncpy(pPos, pString, newLen);
        addSize(newLen);
        return *this;
    }
    inline const Buffer& Buffer::operator+=(const char* pString) {
        addString(pString);
        return *this;
    }
    template<typename T>
    inline const Buffer& Buffer::operator+=(const T& rhs) {
        if(getThread().push(rhs)) {
            getThread().pushAsString(-1);
            getThread().remove(-2);
            addValue();
        }
        return *this;
    }

    template<typename T>
    inline Buffer& Buffer::operator <<(T&& value) {
        *this += value;
        return *this;
    }

    inline bool Buffer::operator==(const char* pStr) {
        assert(isValid());
        return strncmp(getAddress(), pStr, getLength());
    }
    inline bool Buffer::operator!=(const char* pStr) { return !(*this == pStr); }

    template<typename... Args>
    inline Buffer& sprintf(const char* pFormatter, Args&&... args) {
        auto startPos = pos();

        auto argTuple = std::tuple<Args...>(args...);
        const char* lastSource = pFormatter;

        auto addSingle = [&](auto index) {
            addGSub(lastSource, )
        }
    }
}

#endif // ELYSIAN_LUA_BUFFER_HPP
