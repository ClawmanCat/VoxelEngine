#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/serialize/binary_serializable.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>


namespace ve {
    namespace detail {
        // If derived is string_serializable, it can be made automatically binary_serializable
        // by casting the resulting string to a byte array.
        template <typename Derived> struct binary_serializable_from_string {
            [[nodiscard]] std::vector<u8> to_bytes(void) const {
                auto string = static_cast<const Derived*>(this)->to_string();
        
                return std::vector<u8> {
                    ((u8*) string.c_str()),
                    ((u8*) string.c_str() + string.length() + 1) // Include null-terminator.
                };
            }
    
            static Derived from_bytes(std::span<u8> bytes) {
                auto string = std::string {
                    (const char*) &bytes.front(),
                    (const char*) &bytes.back()
                };
        
                return Derived::from_string(std::move(string));
            }
        };
    }
    
    
    template <typename Derived, bool AutomaticBinarySerialization = true>
    struct string_serializable : public std::conditional_t<
        AutomaticBinarySerialization,
        detail::binary_serializable_from_string<Derived>,
        meta::null_type
    > {
        using string_serializable_tag = void;
        
        [[nodiscard]] std::string to_string(void) const {
            VE_CRTP_CHECK(Derived, to_string);
            return static_cast<const Derived*>(this)->to_string();
        }
        
        static Derived from_string(const std::string& str) {
            VE_STATIC_CRTP_CHECK(Derived, from_string);
            return Derived::from_string(str);
        }
    };
}