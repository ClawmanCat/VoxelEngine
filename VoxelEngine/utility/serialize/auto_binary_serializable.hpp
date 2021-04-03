#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/serialize/binary_serializable.hpp>
#include <VoxelEngine/utility/serialize/auto_binary_serializers.hpp>


namespace ve {
    // A class T can extend auto_binary_serializable, if at least one of the following is true:
    // - The class is trivial.
    // - There exists a class template specialization serializer<X> in the namespace ve::serializers,
    //   containing two static methods, to_bytes(const X&) -> std::vector<u8> and from_bytes(std::span<u8>) -> X,
    //   where X will accept a parameter of type T. (X does not have to be T, a constraint can also be used.)
    // - For every member M of T, at least one of the following is true:
    //        - M is auto_binary_serializable.
    //        - M extends binary_serializable.
    // If this requirement is met, extending auto_binary_serializable will generate an implementation of the interface
    // binary_serializable. auto_binary_serializable however does not derive from that interface, so classes that are
    // already marked binary_serializable can decide themselves if this can be done automatically.
    template <typename Derived>
    requires requires { serializers::serializer<Derived>{}; }
    struct auto_binary_serializable {
        std::vector<u8> to_bytes(void) const {
            return serializers::serializer<Derived>::to_bytes((Derived&) *this);
        }
        
        static Derived from_bytes(std::span<u8> bytes) {
            return serializers::serializer<Derived>::from_bytes(bytes);
        }
    };
}