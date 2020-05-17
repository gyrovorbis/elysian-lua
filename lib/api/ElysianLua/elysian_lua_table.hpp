#ifndef ELYSIAN_LUA_TABLE_HPP
#define ELYSIAN_LUA_TABLE_HPP

#include "elysian_lua_object.hpp"

namespace elysian::lua {
//Global operators for shit
template<typename RefType, bool Globals, typename RType>
auto operator+(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::Add); }

template<typename RefType, bool Globals, typename RType>
auto operator-(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::Sub); }

template<typename RefType, bool Globals, typename RType>
auto operator*(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::Mul); }

template<typename RefType, bool Globals, typename RType>
auto operator/(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::Div); }

template<typename RefType, bool Globals, typename RType>
auto operator%(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::Mod); }

template<typename RefType, bool Globals, typename RType>
auto operator&(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::BAnd); }

template<typename RefType, bool Globals, typename RType>
auto operator|(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::BOr); }

#if 0
template<typename RefType, bool Globals, typename RType>
auto operator~(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::BXOr); }
#endif

template<typename RefType, bool Globals, typename RType>
auto operator<<(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::Shl); }

template<typename RefType, bool Globals, typename RType>
auto operator>>(TableBase<RefType, Globals>& lhs, RType&& rhs) { return OperatorProxy(lhs, rhs, OperatorType::Shr); }

template<typename RefType, bool GlobalsTable=false>
class TableBase:
        public Object<RefType>,
        public Callable<TableBase<RefType, GlobalsTable>>
{
public:

    TableBase(const ThreadViewBase* pThread=nullptr);
    // ~TableBase(void); This has to explicitly call m_ref.destroy(getThread()) if not using a stateful reference!!!

    template<typename RefType2, bool GlobalsTable2>
    TableBase(const TableBase<RefType2, GlobalsTable2>& other);

    template<typename RefType2, bool GlobalsTable2>
    TableBase(TableBase<RefType2, GlobalsTable2>&& other);

    template<typename T, typename Key>
    TableBase(const TableProxy<T, Key>& proxy);

    //=====Callable CRTP Overrides=====
    int validateFunc(void) const;
    void pushFunc(void) const;
    //=================================

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

    int getType(void) const;

    template<typename F>
    void iterate(F&& iterator) const;

    template<typename K>
    TableProxy<TableBase<RefType, GlobalsTable>, std::tuple<K>>
    operator[](K key) const;

    operator bool() const;

    template<typename RefType2, bool GlobalsTable2>
    bool operator==(const TableBase<RefType2, GlobalsTable2>& rhs) const;

    template<typename T, typename K>
    bool operator==(const TableProxy<T, K>& rhs) const;

    bool operator==(std::nullptr_t) const;

    template<typename RefType2, bool GlobalsTable2>
    bool operator!=(const TableBase<RefType2, GlobalsTable2>& rhs) const;

    template<typename T, typename K>
    bool operator!=(const TableProxy<T, K>& rhs) const;

    bool operator!=(std::nullptr_t) const;

    template<typename RefType2, bool GlobalsTable2>
    const TableBase<RefType, GlobalsTable>& operator=(const TableBase<RefType2, GlobalsTable2>& rhs);

    template<typename RefType2, bool GlobalsTable2>
    const TableBase<RefType, GlobalsTable>& operator=(TableBase<RefType2, GlobalsTable2>&& rhs);

    template<typename T, typename Key>
    const TableBase<RefType, GlobalsTable>& operator=(const TableProxy<T, Key>& proxy);

    const TableBase<RefType, GlobalsTable>& operator=(std::nullptr_t);

    template<typename... Args>
    const TableBase<RefType, GlobalsTable>& operator+=(const LuaTableValues<Args...>& values) const;

    template<typename RefType2, bool GlobalsTable2>
    const TableBase<RefType, GlobalsTable>& operator+=(const TableBase<RefType2, GlobalsTable2>& rhs) const;

    bool isValid(void) const;
};




template<typename RefType, bool GlobalsTable>
inline TableBase<RefType, GlobalsTable>::TableBase(const ThreadViewBase* pThread):
    Object<RefType>(pThread) {}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline TableBase<RefType, GlobalsTable>::TableBase(const TableBase<RefType2, GlobalsTable2>& other):
    Object<RefType>(other.getThread())
{
    *this = other;
}

template<typename RefType, bool GlobalsTable>
template<typename T, typename Key>
inline TableBase<RefType, GlobalsTable>::TableBase(const TableProxy<T, Key>& proxy):
    Object<RefType>(proxy.getThread())
{
    *this = proxy;
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline TableBase<RefType, GlobalsTable>::TableBase(TableBase<RefType2, GlobalsTable2>&& other):
    Object<RefType>(other.getThread())
{
    *this = std::move(other);
}

template<typename RefType, bool GlobalsTable>
inline int TableBase<RefType, GlobalsTable>::validateFunc(void) const {
    return isValid()? LUA_OK : LUA_ERRRUN;
}

template<typename RefType, bool GlobalsTable>
inline void TableBase<RefType, GlobalsTable>::pushFunc(void) const {
    getRef().push(getThread());
}

template<typename RefType, bool GlobalsTable>
template<typename M>
inline int TableBase<RefType, GlobalsTable>::setMetaTable(M&& meta) const {
    int retVal = 0;
    const int stackIndex = getRef().makeStackIndex();
    if(getThread()->push(std::forward<M>(meta))) {
        retVal = getThread()->setMetaTable(stackIndex);
    }

    getRef().doneWithStackIndex(stackIndex);
    return retVal;
}

template<typename RefType, bool GlobalsTable>
template<typename M>
inline M TableBase<RefType, GlobalsTable>::getMetaTable(void) const {
    M meta;
    getRef().push(getThread());
    if(getThread()->getMetaTable(-1)) {
        getThread()->pull(meta);
    }
    getThread()->pop();
    return meta;
}

template<typename RefType, bool GlobalsTable>
inline TableBase<RefType, GlobalsTable>::operator bool() const {
    return isValid();
}

template<typename RefType, bool GlobalsTable>
inline bool TableBase<RefType, GlobalsTable>::isValid(void) const {
    return getRef().isValid();
}

template<typename RefType, bool GlobalsTable>
inline lua_Integer TableBase<RefType, GlobalsTable>::getLength(void) const {
    const int stackIndex = getRef().makeStackIndex();
    lua_Integer length = getThread()->length(stackIndex);
    getRef().doneWithStackIndex(stackIndex);
    return length;
}

template<typename RefType, bool GlobalsTable>
inline lua_Unsigned TableBase<RefType, GlobalsTable>::getLengthRaw(void) const {
    const int stackIndex = getRef().makeStackIndex();
    lua_Unsigned length = getThread()->lengthRaw(stackIndex);
    getRef().doneWithStackIndex(stackIndex);
    return length;
}

template<typename RefType, bool GlobalsTable>
inline int TableBase<RefType, GlobalsTable>::getType(void) const {
    int type = LUA_TNONE;
    if(getThread()) {
        const int index = getRef().makeStackIndex();
        type = getThread()->getType(index);
        getRef().doneWithStackIndex(index);
    }
    return type;
}

template<typename RefType, bool GlobalsTable>
template<typename K>
inline int TableBase<RefType, GlobalsTable>::getField(K&& key) const {
    if constexpr(GlobalsTable) {
        return getThread()->getGlobalsTable(std::forward<K>(key));
    } else {
        const int stackIndex = getRef().makeStackIndex();
        const int retVal = getThread()->getTable(stackIndex, std::forward<K>(key));
        if constexpr(!RefType::onStack()) {
            getThread()->remove(stackIndex);
        }
        return retVal;
    }
}


template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline int TableBase<RefType, GlobalsTable>::getField(K&& key, V& value) const {
    if constexpr(GlobalsTable) {
        return getThread()->getGlobalsTable(std::forward<K>(key), value);
    } else {
        const int stackIndex = getRef().makeStackIndex();
        const int retVal = getThread()->getTable(stackIndex, std::forward<K>(key), value);
        getRef().doneWithStackIndex(stackIndex);
        return retVal;
    }
}

template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline int TableBase<RefType, GlobalsTable>::getFieldRaw(K&& key, V& value) const {
    const int stackIndex = getRef().makeStackIndex();
    const int retVal = getThread()->getTableRaw(stackIndex, std::forward<K>(key), value);
    getRef().doneWithStackIndex(stackIndex);
    return retVal;
}

template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline void TableBase<RefType, GlobalsTable>::setField(K&& key, V&& value) const {
    if constexpr(GlobalsTable) {
        getThread()->setGlobalsTable(std::forward<K>(key), std::forward<V>(value));
    } else {
        const int stackIndex = getRef().makeStackIndex();
        getThread()->setTable(stackIndex, std::forward<K>(key), std::forward<V>(value));
        getRef().doneWithStackIndex(stackIndex);
    }
}

template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline void TableBase<RefType, GlobalsTable>::setFieldRaw(K&& key, V&& value) const {
    const int stackIndex = getRef().makeStackIndex();
    getThread()->setTableRaw(stackIndex, std::forward<K>(key), std::forward<V>(value));
    getRef().doneWithStackIndex(stackIndex);
}

template<typename RefType, bool GlobalsTable>
template<typename F>
void TableBase<RefType, GlobalsTable>::iterate(F&& iterator) const {
    const int stackIndex = getRef().makeStackIndex();
    getThread()->iterateTable(stackIndex, std::forward<F>(iterator));
    getRef().doneWithStackIndex(stackIndex);
}

template<typename RefType, bool GlobalsTable>
template<typename K>
inline TableProxy<TableBase<RefType, GlobalsTable>, std::tuple<K>>
TableBase<RefType, GlobalsTable>::operator[](K key) const
{
    return TableProxy<TableBase<RefType, GlobalsTable>, std::tuple<K>>(*this, std::make_tuple(key));
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline bool TableBase<RefType, GlobalsTable>::operator==(const TableBase<RefType2, GlobalsTable2>& rhs) const {
    bool result = false;
    lua_State* pLState = getThread()? getThread()->getState() : nullptr;
    lua_State* pRState = rhs.getThread()? rhs.getThread()->getState() : nullptr;

    if(pLState == pRState) {
        if constexpr(GlobalsTable && GlobalsTable2) {
            result = true;
        } else {

            if(isValid() && rhs.isValid()) {
                // Do fast value-check
                if constexpr(std::is_same_v<RefType, RefType2>) {
                    if(getRef() == rhs.getRef()) {
                        return true;
                    }
                }

                const int stackIndex1 = getRef().makeStackIndex();
                const int stackIndex2 = rhs.getRef().makeStackIndex();
                result = getThread()->compare(stackIndex1, stackIndex2, LUA_OPEQ);
                rhs.getRef().doneWithStackIndex(stackIndex2);
                getRef().doneWithStackIndex(stackIndex1);
            } else if(!isValid() && !rhs.isValid()) {
                result = true;
            }
        }
    }
    return result;
}

template<typename RefType, bool GlobalsTable>
template<typename T, typename K>
inline bool TableBase<RefType, GlobalsTable>::operator==(const TableProxy<T, K>& rhs) const {
    bool equal = false;
    if(getThread() == rhs.getThread()) {
        const int index1 = getRef().makeStackIndex();

        if(getThread()->push(rhs)) {
            equal = getThread()->compare(index1, -1, LUA_OPEQ);
            getThread()->pop();
        }

        getRef().doneWithStackIndex(index1);
    }
    return equal;
}

template<typename RefType, bool GlobalsTable>
inline bool TableBase<RefType, GlobalsTable>::operator==(std::nullptr_t) const {
    return !isValid();
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline bool TableBase<RefType, GlobalsTable>::operator!=(const TableBase<RefType2, GlobalsTable2>& rhs) const {
    return !(*this == rhs);
}

template<typename RefType, bool GlobalsTable>
template<typename T, typename K>
inline bool TableBase<RefType, GlobalsTable>::operator!=(const TableProxy<T, K>& rhs) const {
    return !(*this == rhs);
}

template<typename RefType, bool GlobalsTable>
inline bool TableBase<RefType, GlobalsTable>::operator!=(std::nullptr_t) const {
    return isValid();
}


template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator=(const TableBase<RefType2, GlobalsTable2>& rhs) {
    static_assert(!GlobalsTable || GlobalsTable2, "Cannot assign a non-globals table to a globals table!");
    static_assert(!(RefType::onStack() && !RefType2::onStack()), "Cannot assign a stack reference to non-stack object!");

    if(rhs.isValid()) {
        //Let the reference decide how to handle it
        if constexpr(std::is_same_v<RefType, RefType2>) {
            getRef().copy(rhs.getThread(), rhs.getRef());
        } else if constexpr(RefType::onStack() && RefType2::onStack()) {
            getRef().fromStackIndex(rhs.getThread(), rhs.getRef().getIndex());
        } else { // Manually copy via stack
            getThread()->push(rhs);
            getRef().pull(getThread());
        }
    } else {
        getRef().destroy(getThread());
    }
    return *this;
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator=(TableBase<RefType2, GlobalsTable2>&& rhs) {
    static_assert(!GlobalsTable || GlobalsTable2, "Cannot assign a non-globals table to a globals table!");
    static_assert(!(RefType::onStack() && !RefType2::onStack()), "Cannot assign a stack reference to non-stack object!");

    if(rhs.isValid()) {
        //Let the reference decide how to handle it
        if constexpr(std::is_same_v<RefType, RefType2>) {
            getRef().copy(rhs.getThread(), std::move(rhs.getRef()));
        } else if constexpr(RefType::onStack() && RefType2::onStack()) {
            getRef().fromStackIndex(rhs.getThread(), rhs.getRef().getIndex());
        } else { // Manually copy via stack
            getThread()->push(rhs);
            getRef().pull(getThread());
        }
    } else {
        getRef().destroy(getThread());
    }
    return *this;
}

template<typename RefType, bool GlobalsTable>
const TableBase<RefType, GlobalsTable>& TableBase<RefType, GlobalsTable>::operator=(std::nullptr_t) {
    getRef().release();
    return *this;
}

template<typename RefType, bool GlobalsTable>
template<typename T, typename Key>
inline const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator=(const TableProxy<T, Key>& proxy) {
      static_assert(!RefType::onStack(), "Cannot assign a stack reference to non-stack object!");

    if(proxy.isValid()) {
        if(proxy.getThread()->push(proxy)) {
            getRef().pull(proxy.getThread());
        }
    } else {
        getRef().destroy(getThread());
    }
    return *this;
}

template<typename RefType, bool GlobalsTable>
template<typename... Args>
const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator+=(const LuaTableValues<Args...>& values) const {
    const int stackIndex = getRef().makeStackIndex();
    getThread()->appendTable(stackIndex, values);
    getRef().doneWithStackIndex(stackIndex);
    return *this;
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator+=(const TableBase<RefType2, GlobalsTable2>& rhs) const {

    if(isValid() && rhs.isValid()) {
        const int dstTableIndex = getRef().makeStackIndex();

        auto copyTableEntries = [&](auto Fn) {
            rhs.iterate([&]() {
                rhs.getThread()->pushValue(-2); // duplicate key
                rhs.getThread()->pushValue(-2); // duplicate value

                Fn();

                getThread()->setTable(dstTableIndex);

            });
        };

        if(getThread() == rhs.getThread()) {
            copyTableEntries([](){});
        } else {
            copyTableEntries([&](){
                rhs.getThread()->xmove(getThread()->getState(), 2);
            });
        }

        getRef().doneWithStackIndex(dstTableIndex);
    }

    return *this;
}


namespace stack_impl {

struct table_stack_checker {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isTable(index);
    }
};

template<>
struct stack_checker<Table>:
        public table_stack_checker {};

template<>
struct stack_getter<Table>:
        public object_stack_getter<Table> {};

template<>
struct stack_pusher<Table>:
        public object_stack_pusher<Table> {};

template<>
struct stack_checker<StaticTable>:
        public table_stack_checker {};

template<>
struct stack_getter<StaticTable>:
        public object_stack_getter<StaticTable> {};

template<>
struct stack_pusher<StaticTable>:
        public object_stack_pusher<StaticTable> {};

template<>
struct stack_checker<StackTable>:
        public table_stack_checker {};

template<>
struct stack_getter<StackTable>:
        public object_stack_getter<StackTable> {};

template<>
struct stack_pusher<StackTable>:
        public object_stack_pusher<StackTable> {};


template<>
struct stack_checker<StaticStackTable>:
        public table_stack_checker {};

template<>
struct stack_getter<StaticStackTable>:
        public object_stack_getter<StackTable> {};

template<>
struct stack_pusher<StaticStackTable>:
        public object_stack_pusher<StackTable> {};

template<>
struct stack_pusher<GlobalsTable>:
        public object_stack_pusher<GlobalsTable> {};

}



}

#endif // ELYSIAN_LUA_TABLE_HPP
