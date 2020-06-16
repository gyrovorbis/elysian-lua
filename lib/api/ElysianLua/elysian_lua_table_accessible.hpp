#ifndef ELYSIAN_LUA_TABLE_ACCESSIBLE_HPP
#define ELYSIAN_LUA_TABLE_ACCESSIBLE_HPP

#include "elysian_lua_table_proxy.hpp"

namespace elysian::lua {

template<typename CRTP, bool Globals=false>
class TableAccessible {
public:
    template<typename M>
    int setMetaTable(M&& meta) const;

    template<typename M>
    M getMetaTable(void) const;

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

    lua_Integer getLength(void) const;
    lua_Unsigned getLengthRaw(void) const;

    template<typename F>
    void iterate(F&& iterator) const;

    template<typename K>
    TableProxy<CRTP, std::tuple<K>>
    operator[](K key) const;

    template<typename... Args>
    const CRTP& operator+=(const LuaTableValues<Args...>& values) const;

    template<typename T, bool G2>
    std::enable_if_t<std::is_base_of_v<TableAccessible<T, G2>, T>, const CRTP&>
    operator+=(const TableAccessible<T, G2>& rhs) const;

private:

    const CRTP* asDerived(void) const;
    const ThreadViewBase* _getThread(void) const;
    int _makeStackIndex(void) const;
    void _doneWithStackndex(int index) const;
    bool _isValid(void) const;
};

template<typename CRTP, bool Globals>
const CRTP* TableAccessible<CRTP, Globals>::asDerived(void) const {
    return static_cast<const CRTP*>(this);
}

template<typename CRTP, bool Globals>
inline const ThreadViewBase* TableAccessible<CRTP, Globals>::_getThread(void) const {
    return asDerived()->getThread();
}

template<typename CRTP, bool Globals>
inline int TableAccessible<CRTP, Globals>::_makeStackIndex(void) const {
    return asDerived()->makeStackIndex();
}

template<typename CRTP, bool Globals>
inline void TableAccessible<CRTP, Globals>::_doneWithStackndex(int index) const {
    asDerived()->doneWithStackIndex(index);
}

template<typename CRTP, bool Globals>
inline bool TableAccessible<CRTP, Globals>::_isValid(void) const {
    return asDerived()->isValid();
}

template<typename CRTP, bool Globals>
template<typename M>
inline int TableAccessible<CRTP, Globals>::setMetaTable(M&& meta) const {
    int retVal = 0;
    const int stackIndex = this->_makeStackIndex();
    if(this->_getThread()->push(std::forward<M>(meta))) {
        retVal = this->_getThread()->setMetaTable(stackIndex);
    }

    this->_doneWithStackndex(stackIndex);
    return retVal;
}

template<typename CRTP, bool Globals>
template<typename M>
inline M TableAccessible<CRTP, Globals>::getMetaTable(void) const {
    M meta;
    const int stackIndex = this->_makeStackIndex();
    if(this->_getThread()->getMetaTable(stackIndex)) {
        this->_getThread()->pull(meta);
    }
    this->_doneWithStackndex(stackIndex);
    return meta;
}

template<typename CRTP, bool Globals>
inline lua_Integer TableAccessible<CRTP, Globals>::getLength(void) const {
    const int stackIndex = this->_makeStackIndex();
    lua_Integer length = this->_getThread()->length(stackIndex);
    this->_doneWithStackndex(stackIndex);
    return length;
}

template<typename CRTP, bool Globals>
inline lua_Unsigned TableAccessible<CRTP, Globals>::getLengthRaw(void) const {
    const int stackIndex = this->_makeStackIndex();
    lua_Integer length = this->_getThread()->lengthRaw(stackIndex);
    this->_doneWithStackndex(stackIndex);
    return length;
}

template<typename CRTP, bool Globals>
template<typename K>
inline int TableAccessible<CRTP, Globals>::getField(K&& key) const {
    if constexpr(Globals) {
        return this->_getThread()->getGlobalsTable(std::forward<K>(key));
    } else {
        const int stackIndex = this->_makeStackIndex();
        const int retVal = this->_getThread()->getTable(stackIndex, std::forward<K>(key));
        this->_doneWithStackndex(stackIndex);
        return retVal;
    }
}


template<typename CRTP, bool Globals>
template<typename K, typename V>
inline int TableAccessible<CRTP, Globals>::getField(K&& key, V& value) const {
    if constexpr(Globals) {
        return this->_getThread()->getGlobalsTable(std::forward<K>(key), value);
    } else {
        const int stackIndex = this->_makeStackIndex();
        const int retVal = this->_getThread()->getTable(stackIndex, std::forward<K>(key), value);
        this->_doneWithStackndex(stackIndex);
        return retVal;
    }
}

template<typename CRTP, bool Globals>
template<typename K, typename V>
inline int TableAccessible<CRTP, Globals>::getFieldRaw(K&& key, V& value) const {
    const int stackIndex = this->_makeStackIndex();
    const int retVal = this->_getThread()->getTableRaw(stackIndex, std::forward<K>(key), value);
    this->_doneWithStackndex(stackIndex);
    return retVal;
}

template<typename CRTP, bool Globals>
template<typename K, typename V>
inline void TableAccessible<CRTP, Globals>::setField(K&& key, V&& value) const {
    if constexpr(Globals) {
        this->_getThread()->setGlobalsTable(std::forward<K>(key), std::forward<V>(value));
    } else {
        const int stackIndex = this->_makeStackIndex();
        this->_getThread()->setTable(stackIndex, std::forward<K>(key), std::forward<V>(value));
        this->_doneWithStackndex(stackIndex);
    }
}

template<typename CRTP, bool Globals>
template<typename K, typename V>
inline void TableAccessible<CRTP, Globals>::setFieldRaw(K&& key, V&& value) const {
    const int stackIndex = this->_makeStackIndex();
    this->_getThread()->setTableRaw(stackIndex, std::forward<K>(key), std::forward<V>(value));
    this->_doneWithStackndex(stackIndex);
}

template<typename CRTP, bool Globals>
template<typename F>
void TableAccessible<CRTP, Globals>::iterate(F&& iterator) const {
    const int stackIndex = this->_makeStackIndex();
    this->_getThread()->iterateTable(stackIndex, std::forward<F>(iterator));
    this->_doneWithStackndex(stackIndex);
}

template<typename CRTP, bool Globals>
template<typename K>
inline TableProxy<CRTP, std::tuple<K>>
TableAccessible<CRTP, Globals>::operator[](K key) const
{
    return TableProxy<CRTP, std::tuple<K>>(*asDerived(), std::make_tuple(key));
}

template<typename CRTP, bool Globals>
template<typename... Args>
const CRTP&
TableAccessible<CRTP, Globals>::operator+=(const LuaTableValues<Args...>& values) const {
    const int stackIndex = this->_makeStackIndex();
    this->_getThread()->appendTable(stackIndex, values);
    this->_doneWithStackndex(stackIndex);
    return *asDerived();
}

template<typename CRTP, bool Globals>
template<typename T, bool G2>
std::enable_if_t<std::is_base_of_v<TableAccessible<T, G2>, T>, const CRTP&>
TableAccessible<CRTP, Globals>::operator+=(const TableAccessible<T, G2>& rhs) const {

    if(this->_isValid() && rhs._isValid()) {
        int dstTableIndex = this->_makeStackIndex();

        auto copyTableEntries = [&](auto Fn) {
            rhs.iterate([&]() {
                this->_getThread()->pushValue(-2); // duplicate key
                this->_getThread()->pushValue(-2); // duplicate value

                Fn();
                this->_getThread()->setTable(dstTableIndex);

            });
        };

        if(this->_getThread() == rhs._getThread()) {
            copyTableEntries([](){});
        } else {
            copyTableEntries([&](){
                rhs._getThread()->xmove(this->_getThread()->getState(), 2);
            });
        }

        this->_doneWithStackndex(dstTableIndex);
    }

    return *asDerived();
}


}

#endif // ELYSIAN_LUA_TABLE_ACCESSIBLE_HPP
