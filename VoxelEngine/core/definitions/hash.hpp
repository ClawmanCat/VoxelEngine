#pragma once

#include <absl/hash/hash.h>
#include <boost/container_hash/hash.hpp>

#include <concepts>
#include <string_view>


namespace ve {
    namespace detail::hashing_modes {
        template <typename T> concept member_fn_hashable = requires (T value) { { value.hash()          } -> std::convertible_to<std::size_t>; };
        template <typename T> concept static_fn_hashable = requires (T value) { { T::hash(value)        } -> std::convertible_to<std::size_t>; };
        template <typename T> concept adl_fn_hashable    = requires (T value) { { hash(value)           } -> std::convertible_to<std::size_t>; };
        template <typename T> concept abseil_hashable    = requires (T value) { { absl::Hash<T>(value)  } -> std::convertible_to<std::size_t>; };
        template <typename T> concept std_hash_hashable  = requires (T value) { { std::hash<T>{}(value) } -> std::convertible_to<std::size_t>; };
    }


    /** Concept for types hashable according to any of the methods known to the engine (See detail::hashing_modes in core/definitions/hash.hpp). */
    template <typename T> concept hashable =
        detail::hashing_modes::member_fn_hashable<T> ||
        detail::hashing_modes::static_fn_hashable<T> ||
        detail::hashing_modes::adl_fn_hashable<T>    ||
        detail::hashing_modes::abseil_hashable<T>    ||
        detail::hashing_modes::std_hash_hashable<T>;


    /** Hash object for types hashable by the engine. Will attempt to perform hashing methods defined in detail::hashing_modes in core/definitions/hash.hpp. */
    template <hashable T> struct hasher {
        /**
         * Function call operator. Hashes the given value.
         * @param value The object to hash.
         * @return The hash of the object, using one of the hashing methods defined in detail::hashing_modes in core/definitions/hash.hpp.
         */
        constexpr std::size_t operator()(const T& value) const noexcept {
            if constexpr (detail::hashing_modes::member_fn_hashable<T>) return value.hash();
            if constexpr (detail::hashing_modes::static_fn_hashable<T>) return T::hash(value);
            if constexpr (detail::hashing_modes::adl_fn_hashable<T>   ) return hash(value);
            if constexpr (detail::hashing_modes::abseil_hashable<T>   ) return absl::Hash<T>(value);
            if constexpr (detail::hashing_modes::std_hash_hashable<T> ) return std::hash<T>{}(value);
        }
    };


    /** Returns the hash of the given object using the engine's default hasher. */
    template <typename T> constexpr inline std::size_t hash_of(const T& value) {
        return hasher<T>{}(value);
    }


    /**
     * Hashes the provided objects using the engine's default hasher and combines these hashes into a new hash.
     * @param seed An initial seed to combine with the hashed values.
     * @param values The values to be hash-combined.
     * @return A hash value representing the combined hash of the given values.
     */
    template <typename... Ts> constexpr inline std::size_t hash_combine(std::size_t seed, const Ts&... values) {
        ([&] (const auto& v) {
            boost::hash_combine(seed, hash_of(v));
        }(values), ...);

        return seed;
    }
}