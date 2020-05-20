#ifndef ELYSIAN_LUA_TABLE_HPP
#define ELYSIAN_LUA_TABLE_HPP

#include "elysian_lua_object.hpp"
#include "elysian_lua_callable.hpp"
#include "elysian_lua_operator_proxy.hpp"

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

template<typename RefType, bool GlobalsTable>
class TableBase:
        public Object<RefType>//,
        //public Callable<TableBase<RefType, GlobalsTable>>
{
public:

    TableBase(const ThreadViewBase* pThread=nullptr);
    // ~TableBase(void); This has to explicitly call m_ref.destroy(getThread()) if not using a stateful reference!!!

    template<typename RefType2, bool GlobalsTable2>
    TableBase(const TableBase<RefType2, GlobalsTable2>& other);

    template<typename RefType2, bool GlobalsTable2>
    TableBase(TableBase<RefType2, GlobalsTable2>&& other);

    //template<typename T, typename Key>
    //TableBase(const TableProxy<T, Key>& proxy);

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

    TableBase<RefType, GlobalsTable>& operator=(const TableBase<RefType, GlobalsTable>& rhs) const = delete;

    template<typename RefType2, bool GlobalsTable2>
    const TableBase<RefType, GlobalsTable>& operator=(const TableBase<RefType2, GlobalsTable2>& rhs);

    template<typename RefType2, bool GlobalsTable2>
    const TableBase<RefType, GlobalsTable>& operator=(TableBase<RefType2, GlobalsTable2>&& rhs);

    //template<typename T, typename Key>
    //const TableBase<RefType, GlobalsTable>& operator=(const TableProxy<T, Key>& proxy);

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
/*
template<typename RefType, bool GlobalsTable>
template<typename T, typename Key>
inline TableBase<RefType, GlobalsTable>::TableBase(const TableProxy<T, Key>& proxy):
    Object<RefType>(proxy.getThread())
{
    *this = proxy;
}*/

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
    this->getRef().push(this->getThread());
}

template<typename RefType, bool GlobalsTable>
template<typename M>
inline int TableBase<RefType, GlobalsTable>::setMetaTable(M&& meta) const {
    int retVal = 0;
    const int stackIndex = this->getRef().makeStackIndex();
    if(this->getThread()->push(std::forward<M>(meta))) {
        retVal = this->getThread()->setMetaTable(stackIndex);
    }

    this->getRef().doneWithStackIndex(stackIndex);
    return retVal;
}

template<typename RefType, bool GlobalsTable>
template<typename M>
inline M TableBase<RefType, GlobalsTable>::getMetaTable(void) const {
    M meta;
    this->getRef().push(this->getThread());
    if(this->getThread()->getMetaTable(-1)) {
        this->getThread()->pull(meta);
    }
    this->getThread()->pop();
    return meta;
}

template<typename RefType, bool GlobalsTable>
inline TableBase<RefType, GlobalsTable>::operator bool() const {
    return isValid();
}

template<typename RefType, bool GlobalsTable>
inline bool TableBase<RefType, GlobalsTable>::isValid(void) const {
    return this->getRef().isValid();
}

template<typename RefType, bool GlobalsTable>
inline lua_Integer TableBase<RefType, GlobalsTable>::getLength(void) const {
    const int stackIndex = this->getRef().makeStackIndex();
    lua_Integer length = this->getThread()->length(stackIndex);
    this->getRef().doneWithStackIndex(stackIndex);
    return length;
}

template<typename RefType, bool GlobalsTable>
inline lua_Unsigned TableBase<RefType, GlobalsTable>::getLengthRaw(void) const {
    const int stackIndex = this->getRef().makeStackIndex();
    lua_Unsigned length = this->getThread()->lengthRaw(stackIndex);
    this->getRef().doneWithStackIndex(stackIndex);
    return length;
}

template<typename RefType, bool GlobalsTable>
inline int TableBase<RefType, GlobalsTable>::getType(void) const {
    int type = LUA_TNONE;
    if(this->getThread()) {
        const int index = this->getRef().makeStackIndex();
        type = this->getThread()->getType(index);
        this->getRef().doneWithStackIndex(index);
    }
    return type;
}

template<typename RefType, bool GlobalsTable>
template<typename K>
inline int TableBase<RefType, GlobalsTable>::getField(K&& key) const {
    if constexpr(GlobalsTable) {
        return this->getThread()->getGlobalsTable(std::forward<K>(key));
    } else {
        const int stackIndex = this->getRef().makeStackIndex();
        const int retVal = this->getThread()->getTable(stackIndex, std::forward<K>(key));
        if constexpr(!RefType::onStack()) {
            this->getThread()->remove(stackIndex);
        }
        return retVal;
    }
}


template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline int TableBase<RefType, GlobalsTable>::getField(K&& key, V& value) const {
    if constexpr(GlobalsTable) {
        return this->getThread()->getGlobalsTable(std::forward<K>(key), value);
    } else {
        const int stackIndex = this->getRef().makeStackIndex();
        const int retVal = this->getThread()->getTable(stackIndex, std::forward<K>(key), value);
        this->getRef().doneWithStackIndex(stackIndex);
        return retVal;
    }
}

template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline int TableBase<RefType, GlobalsTable>::getFieldRaw(K&& key, V& value) const {
    const int stackIndex = this->getRef().makeStackIndex();
    const int retVal = this->getThread()->getTableRaw(stackIndex, std::forward<K>(key), value);
    this->getRef().doneWithStackIndex(stackIndex);
    return retVal;
}

template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline void TableBase<RefType, GlobalsTable>::setField(K&& key, V&& value) const {
    if constexpr(GlobalsTable) {
        this->getThread()->setGlobalsTable(std::forward<K>(key), std::forward<V>(value));
    } else {
        const int stackIndex = this->getRef().makeStackIndex();
        this->getThread()->setTable(stackIndex, std::forward<K>(key), std::forward<V>(value));
        this->getRef().doneWithStackIndex(stackIndex);
    }
}

template<typename RefType, bool GlobalsTable>
template<typename K, typename V>
inline void TableBase<RefType, GlobalsTable>::setFieldRaw(K&& key, V&& value) const {
    const int stackIndex = this->getRef().makeStackIndex();
    this->getThread()->setTableRaw(stackIndex, std::forward<K>(key), std::forward<V>(value));
    this->getRef().doneWithStackIndex(stackIndex);
}

template<typename RefType, bool GlobalsTable>
template<typename F>
void TableBase<RefType, GlobalsTable>::iterate(F&& iterator) const {
    const int stackIndex = this->getRef().makeStackIndex();
    this->getThread()->iterateTable(stackIndex, std::forward<F>(iterator));
    this->getRef().doneWithStackIndex(stackIndex);
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
    lua_State* pLState = this->getThread()? this->getThread()->getState() : nullptr;
    lua_State* pRState = rhs.getThread()? rhs.getThread()->getState() : nullptr;

    if(pLState == pRState) {
        if constexpr(GlobalsTable && GlobalsTable2) {
            result = true;
        } else {

            if(isValid() && rhs.isValid()) {
                // Do fast value-check
                if constexpr(std::is_same_v<RefType, RefType2>) {
                    if(this->getRef() == rhs.getRef()) {
                        return true;
                    }
                }

                const int stackIndex1 = this->getRef().makeStackIndex();
                const int stackIndex2 = rhs.getRef().makeStackIndex();
                result = this->getThread()->compare(stackIndex1, stackIndex2, LUA_OPEQ);
                rhs.getRef().doneWithStackIndex(stackIndex2);
                this->getRef().doneWithStackIndex(stackIndex1);
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
    if(this->getThread() == rhs.getThread()) {
        const int index1 = this->getRef().makeStackIndex();

        if(this->getThread()->push(rhs)) {
            equal = this->getThread()->compare(index1, -1, LUA_OPEQ);
            this->getThread()->pop();
        }

        this->getRef().doneWithStackIndex(index1);
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
            this->getRef().copy(rhs.getThread(), rhs.getRef());
        } else if constexpr(RefType::onStack() && RefType2::onStack()) {
            this->getRef().fromStackIndex(rhs.getThread(), rhs.getRef().getIndex());
        } else { // Manually copy via stack
            this->getThread()->push(rhs);
            this->getRef().pull(this->getThread());
        }
    } else {
        this->getRef().destroy(this->getThread());
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
            this->getRef().copy(rhs.getThread(), std::move(rhs.getRef()));
        } else if constexpr(RefType::onStack() && RefType2::onStack()) {
            this->getRef().fromStackIndex(rhs.getThread(), rhs.getRef().getIndex());
        } else { // Manually copy via stack
            this->getThread()->push(rhs);
            this->getRef().pull(this->getThread());
        }
    } else {
        this->getRef().destroy(this->getThread());
    }
    return *this;
}

template<typename RefType, bool GlobalsTable>
const TableBase<RefType, GlobalsTable>& TableBase<RefType, GlobalsTable>::operator=(std::nullptr_t) {
    this->getRef().release();
    return *this;
}
/*
template<typename RefType, bool GlobalsTable>
template<typename T, typename Key>
inline const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator=(const TableProxy<T, Key>& proxy) {
      static_assert(!RefType::onStack(), "Cannot assign a stack reference to non-stack object!");

    if(proxy.isValid()) {
        if(proxy.getThread()->push(proxy)) {
            this->getRef().pull(proxy.getThread());
        }
    } else {
        this->getRef().destroy(this->getThread());
    }
    return *this;
}*/

template<typename RefType, bool GlobalsTable>
template<typename... Args>
const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator+=(const LuaTableValues<Args...>& values) const {
    const int stackIndex = this->getRef().makeStackIndex();
    this->getThread()->appendTable(stackIndex, values);
    this->getRef().doneWithStackIndex(stackIndex);
    return *this;
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator+=(const TableBase<RefType2, GlobalsTable2>& rhs) const {

    if(isValid() && rhs.isValid()) {
        const int dstTableIndex = this->getRef().makeStackIndex();

        auto copyTableEntries = [&](auto Fn) {
            rhs.iterate([&]() {
                rhs.getThread()->pushValue(-2); // duplicate key
                rhs.getThread()->pushValue(-2); // duplicate value

                Fn();

                this->getThread()->setTable(dstTableIndex);

            });
        };

        if(this->getThread() == rhs.getThread()) {
            copyTableEntries([](){});
        } else {
            copyTableEntries([&](){
                rhs.getThread()->xmove(this->getThread()->getState(), 2);
            });
        }

        this->getRef().doneWithStackIndex(dstTableIndex);
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
