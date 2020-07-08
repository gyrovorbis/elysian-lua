#ifndef ELYSIAN_LUA_TABLE_HPP
#define ELYSIAN_LUA_TABLE_HPP

#include "elysian_lua_object.hpp"
#include "elysian_lua_callable.hpp"
#include "elysian_lua_operator_proxy.hpp"
#include "elysian_lua_table_accessible.hpp"

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
        public RefType,
        public TableAccessible<TableBase<RefType, GlobalsTable>>
        //public Callable<TableBase<RefType, GlobalsTable>>
{
public:

    using ReferenceType = RefType;

    TableBase(const ThreadViewBase* pThread=nullptr);
    // ~TableBase(void); This has to explicitly call m_ref.destroy(getThread()) if not using a stateful reference!!!

    template<typename RefType2, bool GlobalsTable2>
    TableBase(const TableBase<RefType2, GlobalsTable2>& other);

    template<typename RefType2, bool GlobalsTable2>
    TableBase(TableBase<RefType2, GlobalsTable2>&& other);

    //=====Callable CRTP Overrides=====
    int validateFunc(void) const;
    void pushFunc(void) const;
    //=================================

    int getType(void) const;

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

    bool operator==(bool) const = delete;
    bool operator!=(std::nullptr_t) const;

    //TableBase<RefType, GlobalsTable>& operator=(const TableBase<RefType, GlobalsTable>& rhs) const = delete;

    template<typename RefType2, bool GlobalsTable2>
    const TableBase<RefType, GlobalsTable>& operator=(const TableBase<RefType2, GlobalsTable2>& rhs);

    template<typename RefType2, bool GlobalsTable2>
    const TableBase<RefType, GlobalsTable>& operator=(TableBase<RefType2, GlobalsTable2>&& rhs);

    template<typename T, typename Key>
    const TableBase<RefType, GlobalsTable>& operator=(const TableProxy<T, Key>& proxy);

    const TableBase<RefType, GlobalsTable>& operator=(std::nullptr_t);

    bool isValid(void) const;
};




template<typename RefType, bool GlobalsTable>
inline TableBase<RefType, GlobalsTable>::TableBase(const ThreadViewBase* pThread):
    RefType(pThread) {}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline TableBase<RefType, GlobalsTable>::TableBase(const TableBase<RefType2, GlobalsTable2>& other):
    RefType(other.getThread())
{
    *this = other;
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline TableBase<RefType, GlobalsTable>::TableBase(TableBase<RefType2, GlobalsTable2>&& other):
    RefType(other.getThread())
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
inline TableBase<RefType, GlobalsTable>::operator bool() const {
    return isValid() && !static_cast<const RefType*>(this)->isNilRef();
}

template<typename RefType, bool GlobalsTable>
inline bool TableBase<RefType, GlobalsTable>::isValid(void) const {
    return static_cast<const RefType*>(this)->isValid() &&
            getType() == LUA_TTABLE;
}

template<typename RefType, bool GlobalsTable>
inline int TableBase<RefType, GlobalsTable>::getType(void) const {
    int type = LUA_TNONE;
    if(this->getThread()) {
        const int index = this->makeStackIndex();
        type = this->getThread()->getType(index);
        this->doneWithStackIndex(index);
    }
    return type;
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline bool TableBase<RefType, GlobalsTable>::operator==(const TableBase<RefType2, GlobalsTable2>& rhs) const {
    return (static_cast<const RefType&>(*this) == static_cast<const RefType2&>(rhs));
}

template<typename RefType, bool GlobalsTable>
template<typename T, typename K>
inline bool TableBase<RefType, GlobalsTable>::operator==(const TableProxy<T, K>& rhs) const {
    bool equal = false;
    if(this->getThread() == rhs.getThread()) {
        const int index1 = this->makeStackIndex();

        if(this->getThread()->push(rhs)) {
            equal = this->getThread()->compare(index1, -1, LUA_OPEQ);
            this->getThread()->pop();
        }

        this->doneWithStackIndex(index1);
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
    static_assert(!(StackReferenceable<RefType> && !StackReferenceable<RefType2>), "Cannot assign a stack reference to non-stack object!");

    if(rhs.isValid()) {
        //Let the reference decide how to handle it
        if constexpr(std::is_same_v<RefType, RefType2>) {
            static_cast<RefType&>(this) = static_cast<const RefType2&>(rhs);
        } else if constexpr(StackReferenceable<RefType> && StackReferenceable<RefType2>) {
            this->fromStackIndex(rhs.getThread(), rhs.getStackIndex());
        } else { // Manually copy via stack
            this->getThread()->push(rhs);
            this->pull(this->getThread());
        }
    } else {
        this->release(this->getThread());
    }
    return *this;
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator=(TableBase<RefType2, GlobalsTable2>&& rhs) {
    static_assert(!GlobalsTable || GlobalsTable2, "Cannot assign a non-globals table to a globals table!");
    static_assert(!(StackReferenceable<RefType> && !StackReferenceable<RefType2>), "Cannot assign a stack reference to non-stack object!");

    if(rhs.isValid()) {
        //Let the reference decide how to handle it
        if constexpr(std::is_same_v<RefType, RefType2>) {
            this->getRef() = std::move(rhs.getRef());
        } else if constexpr(StackReferenceable<RefType> && StackReferenceable<RefType2>) {
            this->fromStackIndex(rhs.getThread(), rhs.getStackIndex());
        } else { // Manually copy via stack
            this->getThread()->push(rhs);
            this->pull(this->getThread());
        }
    } else {
        this->release(this->getThread());
    }
    return *this;
}

template<typename RefType, bool GlobalsTable>
const TableBase<RefType, GlobalsTable>& TableBase<RefType, GlobalsTable>::operator=(std::nullptr_t) {
    this->release(this->getThread());
    return *this;
}

template<typename RefType, bool GlobalsTable>
template<typename T, typename Key>
inline const TableBase<RefType, GlobalsTable>&
TableBase<RefType, GlobalsTable>::operator=(const TableProxy<T, Key>& proxy) {
      static_assert(!StackReferenceable<RefType>, "Cannot assign a stack reference to non-stack object!");

    if(proxy.isValid()) {
        if(proxy.getThread()->push(proxy)) {
            this->pull(proxy.getThread());
        }
    } else {
        this->release(this->getThread());
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
