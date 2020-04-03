#ifndef ELYSIAN_LUA_PROXY_HPP
#define ELYSIAN_LUA_PROXY_HPP

#include <tuple>

namespace elysian::lua {

class ThreadView;


template<typename CRTP>
class Proxy {
public:

    template<typename T>
    operator T() const;
   // template<typename T>
   // operator T&() const;

    ThreadView* getThread(void) const;

};


template<typename T, typename Key>
class TableProxy: public Proxy<TableProxy<T,Key>> {
public:

    using Parent = Proxy<TableProxy<T,Key>>;

    template<typename K>
    using ChainedTuple = decltype(std::tuple_cat(m_keys, std::tuple<K>()));

    TableProxy(T table, Key key):
        m_key(std::move(key)), m_table(std::move(table)) {}

    template<typename K>
    auto concatTuple(K key) const {
        return std::tuple_cat(m_key, std::tuple(key));
    }

    template <std::size_t O, std::size_t... Is>
    std::index_sequence<(O + Is)...> add_offset(std::index_sequence<Is...>) const {
        return {};
    }

    template<typename First, typename... Rest>
    inline auto clippedIndexSequence(const std::tuple<First, Rest...>& key) const {
        return add_offset<1>(std::index_sequence_for<Rest...>());
    }

    template<typename T>
    T get(void) const {
        T value;
        const int oldTop = getThread()->getTop();
        m_table.getField(std::get<0>(m_key));   //Always do the first one via the table, which is optimized over the generic thread accesses!!
        if constexpr(std::tuple_size<Key>::value > 1) {
            getThread()->getTableMulti(-1, m_key, clippedIndexSequence(m_key));
        }

        getThread()->pull(value);
        getThread()->pop(getThread()->getTop() - oldTop);
        return value;
    }

    template<typename K2>
    auto operator[](K2 key) const {
        return TableProxy<T, decltype(concatTuple(key))>(m_table, concatTuple(key));
    }

    template<typename V>
    const TableProxy<T, Key>& operator=(V&& value) const {
        //Do a direct set in place
        if constexpr(std::tuple_size<Key>::value == 1) {
            m_table.setField(std::get<0>(m_key), std::forward<V>(value));
        } else {
            //Push first value to be optimal
            const int oldTop = getThread()->getTop();
            m_table.getField(std::get<0>(m_key));
            getThread()->setTableMulti(-1, m_key, clippedIndexSequence(m_key), std::forward<V>(value));
            getThread()->pop(getThread()->getTop() - oldTop);
        }
        return *this;
    }

    ThreadView* getThread(void) const { return m_table.getThread(); }

private:
    Key m_key;
    T m_table;
};


template<typename CRTP>
template<typename T>
inline Proxy<CRTP>::operator T() const {
    return static_cast<const CRTP*>(this)->get<T>();
}
/*
template<typename CRTP>
template<typename T>
inline Proxy<CRTP>::operator T&() const {
    return 4;
    //return static_cast<const CRTP*>(this)->get<T&>();
}*/

template<typename CRTP>
inline ThreadView* Proxy<CRTP>::getThread(void) const {
    return static_cast<const CRTP*>(this)->getThread();
}


}

#endif // ELYSIAN_LUA_PROXY_HPP
