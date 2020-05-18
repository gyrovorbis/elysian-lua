#ifndef ELYSIAN_LUA_TABLE_PROXY_HPP
#define ELYSIAN_LUA_TABLE_PROXY_HPP

#include "elysian_lua_proxy.hpp"

namespace elysian::lua {

template<typename T, typename Key>
class TableFieldRef;

template<typename T, typename Key>
class TableProxy: public Proxy<TableProxy<T,Key>> {
    protected:

    class AddressProxy {
    public:

        AddressProxy(TableProxy<T, Key>& proxy):
            m_proxy(proxy) {}

        template<typename NewTable, typename NewKey>
        operator TableFieldRef<NewTable, NewKey>() const {
            return TableFieldRef<NewTable, NewKey>(getProxy().getParent(), getProxy().getLastKey());
        }

        TableProxy<T, Key>& getProxy(void) const { return m_proxy; }


    private:
        TableProxy<T, Key>& m_proxy;

    };
public:

    using Parent = Proxy<TableProxy<T,Key>>;

    template<typename K>
    using ChainedTuple = decltype(std::tuple_cat(m_keys, std::tuple<K>()));

    TableProxy(T table, Key key);

    template<typename K>
    auto concatTuple(K&& key) const;

    template <std::size_t O, std::size_t... Is>
    std::index_sequence<(O + Is)...> add_offset(std::index_sequence<Is...>) const;

    template <class... Args, std::size_t... Is>
    constexpr static auto subTuple(std::tuple<Args...> tp, std::index_sequence<Is...>)
    {
        return std::tuple{std::get<Is>(tp)...};
    }

    template<typename First, typename... Rest>
    inline auto clippedIndexSequence(const std::tuple<First, Rest...>& key) const;

    int push(const ThreadViewBase* pThread) const;

    template<typename T2>
    T2 get(void) const;

    bool isValid(void) const;

    template<typename K2>
    auto operator[](K2&& key) const;

    AddressProxy operator&(void);

    template<typename V>
    const TableProxy<T, Key>& operator=(V&& value) const;

    template<typename R>
    bool operator==(const R& rhs) const;

    template<typename R>
    bool operator!=(const R& rhs) const;

    const ThreadViewBase* getThread(void) const;
    const T& getTable(void) const;
    const Key& getKey(void) const;
    auto getLastKey(void) const {
        return std::get<std::tuple_size<Key>()-1>(m_key);
    }

    auto getParentKey(void) const {
        return subTuple(m_key, std::make_integer_sequence<std::size_t, std::tuple_size<Key>()-1>{});
    }

    auto getParent(void) const {
        if constexpr(!isTopLevel()) {
            return TableProxy<T, decltype(getParentKey())>(getTable(), getParentKey());
        } else {
            return getTable();
        }
    }

    constexpr static bool isTopLevel(void) { return (std::tuple_size<Key>() == 1); }

private:
    Key m_key;
    T m_table;
};

template<typename T, typename Key>
class TableFieldRef:
        public TableProxy<T, std::tuple<Key>>
{
public:

    TableFieldRef(T table, Key key):
        TableProxy<T, std::tuple<Key>>(std::move(table), std::tuple { std::move(key) })
    {}

    const Key& getKey(void) const {
        return this->getLastKey();
    }

    template<typename V>
    const TableFieldRef<T, Key>& operator=(V&& value) const {
        static_cast<const TableProxy<T, std::tuple<Key>>&>(*this) = std::forward<V>(value);
        return *this;
    }
};





template<typename T, typename Key>
inline TableProxy<T, Key>::TableProxy(T table, Key key):
    m_key(std::move(key)), m_table(std::move(table)) {}

template<typename T, typename Key>
template<typename K>
inline auto TableProxy<T, Key>::concatTuple(K&& key) const {
    return std::tuple_cat(m_key, std::tuple(key));
}

template<typename T, typename Key>
template <std::size_t O, std::size_t... Is>
inline std::index_sequence<(O + Is)...> TableProxy<T, Key>::add_offset(std::index_sequence<Is...>) const {
    return {};
}

template<typename T, typename Key>
template<typename First, typename... Rest>
inline auto TableProxy<T, Key>::clippedIndexSequence(const std::tuple<First, Rest...>& key) const {
    return add_offset<1>(std::index_sequence_for<Rest...>());
}

template<typename T, typename Key>
inline int TableProxy<T, Key>::push(const ThreadViewBase* pThread) const {
    m_table.getField(std::get<0>(m_key));   //Always do the first one via the table, which is optimized over the generic thread accesses!!
    if constexpr(std::tuple_size<Key>::value > 1) {
        getThread()->getTableMulti(-1, m_key, clippedIndexSequence(m_key));
        getThread()->remove(-2);
    }
    return 1;
}

template<typename T, typename Key>
template<typename V>
inline V TableProxy<T, Key>::get(void) const {
    V value;
    push(getThread());
    getThread()->pull(value);
    return value;
}

template<typename T, typename Key>
inline bool TableProxy<T, Key>::isValid(void) const {
    return m_table.isValid();
}

template<typename T, typename Key>
template<typename K2>
inline auto TableProxy<T, Key>::operator[](K2&& key) const {
    return TableProxy<T, decltype(concatTuple(key))>(m_table, concatTuple(std::forward<K2>(key)));
}

template<typename T, typename Key>
inline const ThreadViewBase*
TableProxy<T, Key>::getThread(void) const { return m_table.getThread(); }

template<typename T, typename Key>
inline const T& TableProxy<T, Key>::getTable(void) const { return m_table; }

template<typename T, typename Key>
inline const Key& TableProxy<T, Key>::getKey(void) const { return m_key; }

template<typename T, typename Key>
inline auto TableProxy<T, Key>::operator&(void) -> AddressProxy {
    return AddressProxy(*this);
}

template<typename T, typename Key>
template<typename R>
inline bool TableProxy<T, Key>::operator==(const R& rhs) const {
    bool equal = false;
    if(getThread()) {
        if(getThread()->push(rhs)) {
            if(getThread()->push(*this)) {
                equal = getThread()->compare(-1, -2, LUA_OPEQ);
                getThread()->pop();
            }
            getThread()->pop();
        }
    }
    return equal;
}

template<typename T, typename Key>
template<typename V>
inline const TableProxy<T, Key>&
TableProxy<T, Key>::operator=(V&& value) const {
    //Do a direct set in place
    if constexpr(std::tuple_size<Key>::value == 1) {
        m_table.setField(std::get<0>(m_key), std::forward<V>(value));
    } else {
        //Push first value to be optimal
        m_table.getField(std::get<0>(m_key));
        getThread()->setTableMulti(-1, m_key, clippedIndexSequence(m_key), std::forward<V>(value));
        getThread()->pop();
    }
    return *this;
}

template<typename T, typename Key>
template<typename R>
inline bool TableProxy<T, Key>::operator!=(const R& rhs) const {
    return !(*this == rhs);
}

namespace stack_impl {

template<typename T, typename K>
struct stack_pusher<TableProxy<T, K>>:
        public proxy_stack_pusher<TableProxy<T, K>>{};


}

}

#endif // ELYSIAN_LUA_TABLE_PROXY_HPP
