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

struct TableBaseTag {};

template<typename RefType, bool GlobalsTable>
class TableBase:
        TableBaseTag,
        public RefType,
        public TableAccessible<TableBase<RefType, GlobalsTable>>
        //public Callable<TableBase<RefType, GlobalsTable>>
{
public:

    using ReferenceType = RefType;

    static_assert(ReadableReferenceable<RefType>, "RefType must meet all of the constraints required by the ReadableReferenceable concept!");
    static_assert(ThreadStateful<RefType>, "RefType must meet all of the constraints required by the ThreadStateful concept!");

    TableBase(void) = default;
    TableBase(const ThreadViewBase* pThread) requires ModifiableThreadStateful<RefType>;
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

#if 0
    template<typename T, typename K>
    bool operator==(const TableProxy<T, K>& rhs) const;
#endif

    bool operator==(std::nullptr_t) const;

    template<typename RefType2, bool GlobalsTable2>
    bool operator!=(const TableBase<RefType2, GlobalsTable2>& rhs) const;
#if 0
    template<typename T, typename K>
    bool operator!=(const TableProxy<T, K>& rhs) const;
#endif

    bool operator==(bool) const = delete;
    bool operator!=(std::nullptr_t) const;
#if 0
    template<typename T, typename Key>
    const TableBase<RefType, GlobalsTable>& operator=(const TableProxy<T, Key>& proxy);
#endif

    const TableBase<RefType, GlobalsTable>& operator=(std::nullptr_t);

    TableBase<RefType, GlobalsTable>& operator=(auto&& rhs);

    bool isValid(void) const;
};

template<typename RefType, bool GlobalsTable>
inline TableBase<RefType, GlobalsTable>& TableBase<RefType, GlobalsTable>::operator=(auto&& rhs) {
    static_cast<RefType&>(*this) = std::forward<decltype(rhs)>(rhs);
    return *this;
}


template<typename RefType, bool GlobalsTable>
inline TableBase<RefType, GlobalsTable>::TableBase(const ThreadViewBase* pThread)
requires ModifiableThreadStateful<RefType>:
    RefType(pThread)
{}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline TableBase<RefType, GlobalsTable>::TableBase(const TableBase<RefType2, GlobalsTable2>& other):
    RefType(static_cast<const RefType2&>(other))
{
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline TableBase<RefType, GlobalsTable>::TableBase(TableBase<RefType2, GlobalsTable2>&& other):
    RefType(static_cast<RefType2&&>(other))
{
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

#if 0
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
#endif

template<typename RefType, bool GlobalsTable>
inline bool TableBase<RefType, GlobalsTable>::operator==(std::nullptr_t) const {
    return !isValid();
}

template<typename RefType, bool GlobalsTable>
template<typename RefType2, bool GlobalsTable2>
inline bool TableBase<RefType, GlobalsTable>::operator!=(const TableBase<RefType2, GlobalsTable2>& rhs) const {
    return !(*this == rhs);
}

#if 0
template<typename RefType, bool GlobalsTable>
template<typename T, typename K>
inline bool TableBase<RefType, GlobalsTable>::operator!=(const TableProxy<T, K>& rhs) const {
    return !(*this == rhs);
}
#endif

template<typename RefType, bool GlobalsTable>
inline bool TableBase<RefType, GlobalsTable>::operator!=(std::nullptr_t) const {
    return isValid();
}

template<typename RefType, bool GlobalsTable>
const TableBase<RefType, GlobalsTable>& TableBase<RefType, GlobalsTable>::operator=(std::nullptr_t) {
    this->release();
    return *this;
}

#if 0
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
        this->release();
    }
    return *this;
}
#endif

namespace stack_impl {

template<typename RefType, bool GlobalsTable>
    requires (!GlobalsTable)
struct stack_checker<TableBase<RefType, GlobalsTable>> {
    static bool check(const ThreadViewBase* pBase, StackRecord& record, int index) {
        return pBase->isTable(index);
    }
};

}


}

#endif // ELYSIAN_LUA_TABLE_HPP
