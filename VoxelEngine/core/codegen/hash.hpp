#pragma once

#include <VoxelEngine/core/typedef/scalar.hpp>

#include <boost/pfr.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/preprocessor.hpp>
#include <ctti/type_id.hpp>

#include <cstddef>
#include <functional>
#include <type_traits>


namespace ve {
    template <typename T> constexpr inline std::size_t hash_of(const T& value) {
        return std::hash<T>{}(value);
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
template <typename T> requires requires (const T t) { std::cbegin(t), std::cend(t); }
struct std::hash<T> {
    constexpr std::size_t operator()(const T& value) const {
        std::size_t hash = 0;
        for (const auto& elem : value) boost::hash_combine(hash, ve::hash_of(elem));
        return hash;
    }
};


// Make the type automatically hashable
#define ve_hashable()                                               \
constexpr std::size_t hash(void) const {                            \
    std::size_t hash = 0;                                           \
                                                                    \
    boost::pfr::for_each_field(*this, [&](const auto& elem) {       \
        boost::hash_combine(hash, ve::hash_of(elem));               \
    });                                                             \
                                                                    \
    return hash;                                                    \
}


// Make the type automatically hashable on the given fields.
#define ve_impl_hash_field(R, D, E) boost::hash_combine(hash, ve::hash_of(E));

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


namespace ve {
    template <typename T> constexpr u64 type_hash(void) {
        return ctti::unnamed_type_id<T>().hash();
    }


    struct compare_by_hash {
        template <typename T>
        constexpr bool operator()(const T& a, const T& b) const {
            const auto hasher = std::hash<T>{};
            return hasher(a) < hasher(b);
        }
    };
}