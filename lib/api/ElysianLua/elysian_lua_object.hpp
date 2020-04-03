#ifndef ELYSIAN_LUA_OBJECT_HPP
#define ELYSIAN_LUA_OBJECT_HPP

#include <ElysianLua/elysian_lua_thread_view.hpp>
#include <ElysianLua/elysian_lua_proxy.hpp>

namespace elysian::lua {

struct ObjectBase {};

template<typename RefType>
class Object: public ObjectBase {
public:

    Object(ThreadView* pThread);

    ThreadView* getThread(void) const;
    const RefType& getRef(void) const;


protected:

    RefType m_ref;
};



template<typename RefType, bool GlobalsTable=false>
class TableBase: public Object<RefType> {
public:
    TableBase(ThreadView* pThread);

    template<typename K>
    int getField(K&& key) const;

    template<typename K, typename V>
    int getField(K&& key, V& value) const;

    template<typename K, typename V>
    int getFieldRaw(K&& key, V& value) const;

    template<typename K, typename V>
    void setField(K&& key, V&& value) const;

    template<typename K, typename V>
    void setFieldRaw(K&& key, V&& value) const;

    template<typename K>
    TableProxy<TableBase<RefType, GlobalsTable>, std::tuple<K>>
    operator[](K key) const;

};



template<typename RefType>
inline Object<RefType>::Object(ThreadView* pThread): m_ref(pThread) {}

template<typename RefType>
inline const RefType& Object<RefType>::getRef(void) const { return m_ref; }

template<typename RefType>
inline ThreadView* Object<RefType>::getThread(void) const { return m_ref.getThread(); }


template<typename RefType, bool GlobalsTable=false>
inline TableBase<RefType, GlobalsTable>::TableBase(ThreadView* pThread): Object<RefType>(pThread) {}

template<typename RefType, bool GlobalsTable>
template<typename K>
inline int TableBase<RefType, GlobalsTable>::getField(K&& key) const {
    if constexpr(GlobalsTable) {
        return getThread()->getGlobalsTable(std::forward<K>(key));
    } else {
        m_ref.push();
        const int retVal = getThread()->getTable(-1, std::forward<K>(key));
        getThread()->remove(-2);
        return retVal;
    }
}


template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline int TableBase<RefType, GlobalsTable>::getField(K&& key, V& value) const {
    if constexpr(GlobalsTable) {
        return getThread()->getGlobalsTable(std::forward<K>(key), value);
    } else {
        m_ref.push();
        const int retVal = getThread()->getTable(-1, std::forward<K>(key), value);
        pop(1);
        return retVal;
    }
}

template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline int TableBase<RefType, GlobalsTable>::getFieldRaw(K&& key, V& value) const {
    m_ref.push();
    const int retVal = getThread()->getTableRaw(-1, std::forward<K>(key), value);
    pop(1);
    return retVal;
}

template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline void TableBase<RefType, GlobalsTable>::setField(K&& key, V&& value) const {
    if constexpr(GlobalsTable) {
        getThread()->setGlobalsTable(std::forward<K>(key), std::forward<V>(value));
    } else {
        m_ref.push();
        getThread()->setTable(std::forward<K>(key), std::forward<V>(value));
        pop(1);
    }
}

template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline void TableBase<RefType, GlobalsTable>::setFieldRaw(K&& key, V&& value) const {
    m_ref.push();
    getThread()->setTableRaw(std::forward<K>(key), std::forward<V>(value));
    pop(1);
}

template<typename RefType, bool GlobalsTable>
template<typename K>
inline TableProxy<TableBase<RefType, GlobalsTable>, std::tuple<K>> TableBase<RefType, GlobalsTable>::operator[](K key) const {
    return TableProxy<TableBase<RefType, GlobalsTable>, std::tuple<K>>(*this, std::make_tuple(key));
}




}

#endif // ELYSIAN_LUA_OBJECT_HPP
