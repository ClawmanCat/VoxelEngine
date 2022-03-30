#pragma once

#include <VoxelEngine/core/typedef/scalar.hpp>

#include <boost/pfr.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/preprocessor.hpp>
#include <ctti/type_id.hpp>

#include <cstddef>
#include <functional>
#include <type_traits>
#include <filesystem>


namespace ve {
    namespace detail {
        // Returns true if T::iterator::value_type == T.
        template <typename T> constexpr static bool iterates_itself = std::is_same_v<
            std::remove_cvref_t<T>,
            typename std::iterator_traits<std::remove_cvref_t<decltype(std::cbegin(std::declval<T>()))>>::value_type
        >;
    }


    template <typename T> constexpr inline std::size_t hash_of(const T& value) {
        return std::hash<T>{}(value);
    }

    // Drop-in replacement for boost::hash_combine. Prevents boost from complaining that boost::hash isn't defined
    // when we only provide a std::hash overload for the type.
    template <typename T> constexpr inline void hash_combine(std::size_t& seed, const T& value) {
        boost::hash_combine(seed, hash_of(value));
    }

    // Used for serialization so should be an explicitly sized type.
    template <typename T> constexpr u64 type_hash(void) {
        return ctti::unnamed_type_id<T>().hash();
    }
}


// Allow specifying a hash member function instead of overloading std::hash.
template <typename T> requires requires (const T t) { { t.hash() } -> std::convertible_to<std::size_t>; }
struct std::hash<T> {
    constexpr std::size_t operator()(const T& value) const {
        return value.hash();
    }
};

// Allow hashing of containers.
template <typename T> requires (
    requires (const T t) { std::cbegin(t), std::cend(t); } && // Must be iterable
    !ve::detail::iterates_itself<T> // Element type cannot be T, this would cause infinite recursion.
) struct std::hash<T> {
    constexpr std::size_t operator()(const T& value) const {
        std::size_t hash = 0;
        for (const auto& elem : value) ve::hash_combine(hash, ve::hash_of(elem));
        return hash;
    }
};


// Make the type automatically hashable
#define ve_hashable()                                               \
constexpr std::size_t hash(void) const {                            \
    std::size_t hash = 0;                                           \
                                                                    \
    boost::pfr::for_each_field(*this, [&](const auto& elem) {       \
        ve::hash_combine(hash, ve::hash_of(elem));                  \
    });                                                             \
                                                                    \
    return hash;                                                    \
}


// Make the type automatically hashable on the given fields.
#define ve_impl_hash_field(R, D, E) ve::hash_combine(hash, ve::hash_of(E));

#define ve_field_hashable(...)                                      \
constexpr std::size_t hash(void) const {                            \
    std::size_t hash = 0;                                           \
                                                                    \
    BOOST_PP_SEQ_FOR_EACH(                                          \
        ve_impl_hash_field,                                         \
        _,                                                          \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                       \
    );                                                              \
                                                                    \
    return hash;                                                    \
}