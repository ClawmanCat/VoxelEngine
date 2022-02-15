#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::serialize {
    // This class can be overloaded to provide a custom serializer for otherwise non-serializable types.
    // Specializations of this template should implement the following methods:
    //
    // static void to_bytes(const T& value, std::vector<u8>& dest)
    // static T from_bytes(std::span<const u8>& src)
    //
    // to_bytes should append its result to the end of the vector.
    // from_bytes should construct the given object from the end of the span,
    // and reduce the size of the span to remove the bytes that were used for deserialization.
    template <typename T> struct binary_serializer {
        using non_overloaded_tag = void;
    };


    // Alternatively, a class may provide the above methods as class member functions.
    template <typename T> requires requires (T t, std::vector<u8>& dst, std::span<const u8>& src) {
        { t.to_bytes(dst)    } -> std::same_as<void>;
        { T::from_bytes(src) } -> std::same_as<T>;
    } struct binary_serializer<T> {
        static void to_bytes(const T& value, std::vector<u8>& dest) {
            value.to_bytes(dest);
        }

        static T from_bytes(std::span<const u8>& src) {
            return T::from_bytes(src);
        }
    };
}