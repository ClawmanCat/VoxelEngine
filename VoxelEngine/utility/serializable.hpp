#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/logger.hpp>

#include <boost/pfr.hpp>

#include <type_traits>
#include <span>


// TODO: Automatically support container serialization if element is serializable.
namespace ve {
    template <typename Derived> struct binary_serializable {
        using binary_serializable_tag = void;
        
        [[nodiscard]] std::vector<u8> to_bytes(void) const {
            VE_CRTP_CHECK(Derived, to_bytes);
            return static_cast<const Derived*>(this)->to_bytes();
        }
        
        static Derived from_bytes(std::span<u8> bytes) {
            VE_STATIC_CRTP_CHECK(Derived, from_bytes);
            return Derived::from_bytes(bytes);
        }
        
        constexpr static u32 size_estimate(void) {
            // For every member in Derived, if it implements size_estimate, use that,
            // otherwise assume sizeof(Member).
            // If Derived has its own size_estimate, use that instead.
            if constexpr (VE_STATIC_CRTP_IS_IMPLEMENTED(Derived, size_estimate)) {
                return Derived::size_estimate();
            } else {
                std::size_t accumulated_size = 0;
                
                iterate_class_members<Derived>([&] <typename Member> () {
                    if constexpr (requires { Member::size_estimate(); }) {
                        accumulated_size += Member::size_estimate();
                    } else {
                        accumulated_size += sizeof(Member);
                    }
                });
                
                return accumulated_size;
            }
        }
    };
    
    
    namespace detail {
        // Automatically overrides [from|to]_bytes by casting the result of [from|to]_string to a byte array.
        template <typename Derived> struct automatic_binary_serializable_from_string {
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
    
    
    // A type that is string_serializable can be made binary_serializable automatically by simply storing the bytes
    // in the serialized string. Often however, it is preferred to provide a custom binary serialization method,
    // to increase performance and decrease the required storage.
    // The parameter AutomaticBinarySerialization can be used to enable automatic binary serialization.
    template <typename Derived, bool AutomaticBinarySerialization = true>
    struct string_serializable : public std::conditional_t<
        AutomaticBinarySerialization,
        detail::automatic_binary_serializable_from_string<Derived>,
        meta::null_type
    > {
        using binary_serializable_tag = void;
        
        [[nodiscard]] std::string to_string(void) const {
            VE_CRTP_CHECK(Derived, to_string);
            return static_cast<const Derived*>(this)->to_string();
        }
    
        static Derived from_string(const std::string& str) {
            VE_STATIC_CRTP_CHECK(Derived, from_string);
            return Derived::from_string(str);
        }
    };
    
    
    namespace detail {
        constexpr static std::size_t header_size = sizeof(u32);
        
        
        // TODO: Use variable size header and encode header size in MSBs.
        inline std::vector<u8> encode_size_header(std::size_t size) {
            return std::vector<u8> {
                ((u8*) &size) + 0,
                ((u8*) &size) + header_size
            };
        }
        
        
        inline std::size_t decode_size_header(std::span<u8> bytes) {
            u32 result;
            
            // Cannot simply cast, as we have no guarantee alignment requirements will be met.
            memcpy(&result, &bytes[0], header_size);
            
            return (std::size_t) result;
        }
    }
        
    
    // Converts the given object to a byte array by either:
    // - Recursively calling this method on each member if the object is of class type
    // - Casting the object to a byte array if it is of a trivial type.
    // - Using the binary_serializable interface if the class inherits from it.
    // Note: if T derives from binary_serializable, you should call state.reserve(T::size_estimate()) before calling this method to increase performance.
    template <typename T, bool called_recursively = false>
    inline void recursive_serialize(std::vector<u8>& state, const T& current) {
        static_assert(!std::is_pointer_v<T>, "Cannot serialize pointer types.");
        static_assert(!std::is_union_v<T>,   "Cannot serialize union types.");
        static_assert(std::is_trivial_v<T>,  "Cannot serialize non-trivial types.");

        if constexpr (std::is_class_v<T>) {
            // Don't check for binary serialization on the initial call, as the implementation to serialize
            // the current object may be calling this method.
            if constexpr (called_recursively && requires { typename T::binary_serializable_tag; }) {
                auto bytes = current.to_bytes();
                state.insert(state.end(), bytes.begin(), bytes.end());
            } else {
                iterate_class_members(
                    current,
                    [&] (const auto& member) {
                        // We don't know what the actual size of the member is until it is serialized,
                        // so just save some space for the header and fill it in afterwards.
                        // TODO: Account for variable header size if it is implemented.
                        std::size_t header_index = state.size();
                        state.resize(state.size() + detail::header_size);
            
                        recursive_serialize<true>(state, member);
            
                        std::size_t member_size = state.size() - header_index - detail::header_size;
                        auto header = detail::encode_size_header(member_size);
            
                        state.insert(state.begin() + header_index, header.begin(), header.end());
                    }
                );
            }
        } else {
            state.insert(state.end(), ((u8*) &current) + 0, ((u8*) &current) + sizeof(T));
        }
    }
    
    
    template <typename T, bool called_recursively = false>
    inline T recursive_deserialize(std::span<u8> bytes) {
        if constexpr (std::is_class_v<T>) {
            // Don't check for binary serialization on the initial call, as the implementation to deserialize
            // the current object may be calling this method.
            if constexpr (called_recursively && requires { typename T::binary_serializable_tag; }) {
                return T::from_bytes(bytes);
            } else {
                T result;
                
                iterate_class_members(
                    result,
                    [&] <typename Member> (Member& member) {
                        std::size_t member_size = detail::decode_size_header(bytes.first(detail::header_size));
                        bytes = bytes.subspan(detail::header_size);
            
                        member = recursive_deserialize<Member, true>(bytes.subspan(0, member_size));
                        bytes = bytes.subspan(member_size);
                    }
                );
                
                return result;
            }
        } else {
            VE_ASSERT(
                bytes.size() == sizeof(T),
                "Possible data corruption: number of serialized bytes is different from object size."
            );
            
            T result;
            memcpy(&result, &bytes.front(), &bytes.back());
            return result;
        }
    }
    
    
    // For simple types, serialization can be done automatically with recursive_[de]serialize.
    // This class does not extend binary_serializable to allow a class already deriving from binary_serializable
    // to use this class without the requirement of virtual inheritance being enabled everywhere in the inheritance chain.
    template <typename Derived> struct auto_binary_serializable {
        [[nodiscard]] std::vector<u8> to_bytes(void) const {
            std::vector<u8> result;
            
            if constexpr (requires { Derived::size_estimate(); }) result.reserve(Derived::size_estimate());
            recursive_serialize(result, (Derived&) *this);
            
            return result;
        }
    
        static Derived from_bytes(std::span<u8> bytes) {
            return recursive_deserialize<Derived>(bytes);
        }
    };
}