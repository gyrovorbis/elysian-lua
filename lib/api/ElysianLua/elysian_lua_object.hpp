#ifndef ELYSIAN_LUA_OBJECT_HPP
#define ELYSIAN_LUA_OBJECT_HPP

#include "elysian_lua_reference.hpp"

namespace elysian::lua {

class ThreadViewBase;

struct ObjectBase {};

template<typename RefType>
class Object: public ObjectBase {
public:
    using ReferenceType = RefType;

    Object(const ThreadViewBase* pThread);

    bool push(const ThreadViewBase* pThread) const;
    bool fromStackIndex(const ThreadViewBase* pThread, int index);

    const ThreadViewBase* getThread(void) const;
    const RefType &getRef(void) const;
    RefType& getRef(void);

protected:

    RefType m_ref;
};


template<typename RefType>
inline Object<RefType>::Object(const ThreadViewBase* pThread): m_ref(pThread) {}

template<typename RefType>
inline const RefType& Object<RefType>::getRef(void) const { return m_ref; }

template<typename RefType>
inline RefType& Object<RefType>::getRef(void) { return m_ref; }

template<typename RefType>
inline const ThreadViewBase* Object<RefType>::getThread(void) const { return m_ref.getThread(); }

template<typename RefType>
inline bool Object<RefType>::push(const ThreadViewBase* pThread) const {
    return m_ref.push(pThread);
}

template<typename RefType>
inline bool Object<RefType>::fromStackIndex(const ThreadViewBase* pThread, int index) {
    return m_ref.fromStackIndex(pThread, index);
}


namespace stack_impl {

template<typename O>
struct object_stack_getter {
    static O get(const ThreadViewBase* pBase, StackRecord& record, int index) {
        O object(pBase);
        object.fromStackIndex(pBase, index);
        return object;
    }
};

template<typename O>
struct object_stack_pusher {
    static int push(const ThreadViewBase* pBase, StackRecord& record, const O& object) {
        return object.push(pBase);
    }
};

}

}

#endif // ELYSIAN_LUA_OBJECT_HPP
