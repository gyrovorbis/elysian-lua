#ifndef ELYSIAN_LUA_TRAITS_HPP
#define ELYSIAN_LUA_TRAITS_HPP

#include <type_traits>
#include <tuple>

namespace elysian::lua {

template <typename... Args>
    struct types {
        typedef std::make_index_sequence<sizeof...(Args)> indices;
        static constexpr std::size_t size() {
            return sizeof...(Args);
        }
    };

    namespace meta_detail {
        template <typename T, template <typename...> class Templ>
        struct is_specialization_of : std::false_type {};
        template <typename... T, template <typename...> class Templ>
        struct is_specialization_of<Templ<T...>, Templ> : std::true_type {};

        template <typename... Args>
        struct tuple_types_ { typedef types<Args...> type; };

        template <typename... Args>
        struct tuple_types_<std::tuple<Args...>> { typedef types<Args...> type; };

    } // namespace meta_detail

    template <typename T, template <typename...> class Templ>
    using is_specialization_of = meta_detail::is_specialization_of<std::remove_cv_t<T>, Templ>;

    template <typename T, template <typename...> class Templ>
    inline constexpr bool is_specialization_of_v = is_specialization_of<std::remove_cv_t<T>, Templ>::value;

    template <typename T>
    using is_tuple = is_specialization_of<T, std::tuple>;

    template <typename T>
    constexpr inline bool is_tuple_v = is_tuple<T>::value;

    template <typename... Args>
    using tuple_types = typename meta_detail::tuple_types_<Args...>::type;

    template<typename T, typename E>
    using sfinae_decay_type_t = std::enable_if_t<std::is_same_v<std::decay_t<T>, E>, void>;

    //template<typename T, typename... Es>
    //using sfinae_decay_types = std::enable_if_t<(std::is_same<T, std::decay<Es>>) || ...)
}

#ifdef ELYSIAN_LUA_ENABLE_CONCEPTS


    template<typename T, typename U>
    concept same_as = std::is_same_v<T, U> && std::is_same_v<U, T>;
#endif



#endif // ELYSIAN_LUA_TRAITS_HPP
