#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/serialize/overloadable_serializer.hpp>
#include <VoxelEngine/utility/io/serialize/push_serializer.hpp>
#include <VoxelEngine/utility/io/serialize/variable_length_encoder.hpp>
#include <VoxelEngine/utility/io/serialize/decomposable_serializer.hpp>
#include <VoxelEngine/utility/io/serialize/container_serializer.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/traits/evaluate_if_valid.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve::serialize {
    template <typename T> constexpr inline bool is_serializable =
        (std::is_const_v<T> && is_serializable<std::remove_const_t<T>>)    ||
        (!requires { typename binary_serializer<T>::non_overloaded_tag; }) ||
        std::is_trivial_v<T>                                               ||
        (
            detail::supports_container_serialization_v<T> &&
            ve_eval_if_valid(is_serializable<typename T::value_type>)
        )                                                                  ||
        (
            is_decomposable_v<T> &&
            ve_eval_if_valid(meta::create_pack::from_decomposable<T>::all(
                [] <typename M> () { return is_serializable<M>; }
            ))
        );



    template <typename T> void to_bytes(const T& value, std::vector<u8>& dest) {
        // If T is const, use the same serializer as non-const T.
        if constexpr (std::is_const_v<T>) {
            to_bytes<std::remove_const_t<T>>(value, dest);
        }

        // If T has a custom serializer, use it.
        else if constexpr (!requires { typename binary_serializer<T>::non_overloaded_tag; }) {
            binary_serializer<T>::to_bytes(value, dest);
        }

        // If T is trivial, just copy its bytes.
        else if constexpr (std::is_trivial_v<T>) {
            push_serializer ser { dest };
            ser.push(value);
        }

        // If T is a container-like type, serialize each element.
        else if constexpr (detail::supports_container_serialization_v<T>) {
            container_to_bytes(value, dest);
        }

        // If T is decomposable (i.e. it is possible to extract its members through some method like PFR or std::get) serialize each member.
        else if constexpr (is_decomposable_v<T> && ve_eval_if_valid(meta::create_pack::from_decomposable<T>::all([] <typename M> () { return is_serializable<M>; }))) {
            decomposable_to_bytes(value, dest);
        }

        else static_assert(
            meta::always_false_v<T>,
            "No known method exists to serialize this type. "
            "To fix this error, please specialize ve::serialize::binary_serializer for your type."
        );
    }


    // Overload for automatically constructing the destination vector.
    template <typename T> std::vector<u8> to_bytes(const T& value) {
        std::vector<u8> dest;
        to_bytes<T>(value, dest);
        return dest;
    }


    template <typename T> T from_bytes(std::span<const u8>& src) {
        // If T is const, use the same serializer as non-const T.
        if constexpr (std::is_const_v<T>) {
            return from_bytes<std::remove_const_t<T>>(src);
        }

        // If T has a custom serializer, use it.
        else if constexpr (!requires { typename binary_serializer<T>::non_overloaded_tag; }) {
            return binary_serializer<T>::from_bytes(src);
        }

        // If T is trivial, just copy its bytes.
        else if constexpr (std::is_trivial_v<T>) {
            pop_deserializer ser { take_back_n(src, sizeof(T)) };
            return ser.pop<T>();
        }

        // If T is a container-like type, serialize each element.
        else if constexpr (detail::supports_container_serialization_v<T>) {
            return container_from_bytes<T>(src);
        }

        // If T is decomposable (i.e. it is possible to extract its members through some method like PFR or std::get) serialize each member.
        else if constexpr (is_decomposable_v<T> && ve_eval_if_valid(meta::create_pack::from_decomposable<T>::all([] <typename M> () { return is_serializable<M>; }))) {
            return decomposable_from_bytes<T>(src);
        }

        else static_assert(
            meta::always_false_v<T>,
            "No known method exists to serialize this type. "
            "To fix this error, please specialize ve::serialize::binary_serializer for your type."
        );
    }


    // Overload for non-span containers.
    template <typename T, typename Ctr> requires (!std::is_same_v<Ctr, std::span<const u8>>)
    T from_bytes(const Ctr& ctr) {
        std::span<const u8> s { ctr.begin(), ctr.end() };
        return from_bytes<T>(s);
    }
}