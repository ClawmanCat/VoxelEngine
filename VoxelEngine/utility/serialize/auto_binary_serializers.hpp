#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/serialize/serialization_utils.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>

#include <boost/pfr.hpp>


namespace ve::serializers {
    template <typename T> struct serializer {
        static_assert(meta::always_false_v<T>, "Cannot serialize type without a known serializer.");
    };
    
    
    
    
    // Trivial types.
    template <typename T> requires
        std::is_trivial_v<T> &&
        // Can't serialize pointers, since it would slice the data if we don't have a pointer to the most derived class.
        (!std::is_pointer_v<T>)
    struct serializer<T> {
        static std::vector<u8> to_bytes(const T& value) {
            std::vector<u8> result;
            result.resize(sizeof(T));
            
            memcpy(&result[0], &value, sizeof(T));
            
            return result;
        }
        
        static T from_bytes(std::span<u8> bytes) {
            T result;
            memcpy(&result, &bytes[0], sizeof(T));
        }
    };
    
    
    
    
    // Containers.
    template <typename T>
    requires requires (T t, typename T::iterator it, typename T::value_type v) {
        // T must be iteratable.
        std::begin(t),
        std::end(t),
        // There must be a way to construct T.
        T { },
        // Members of T must be serializable.
        serializer<typename T::value_type>{},
        // There must be a way to insert into T.
        t.insert(it, v);
    } struct serializer<T> {
        using elem_serializer_t = serializer<typename T::value_type>;
        
        static std::vector<u8> to_bytes(const T& value) {
            push_serializer serializer { sizeof(T) * std::size(value) };
            
            for (const auto& elem : value) {
                auto bytes = elem_serializer_t::to_bytes(elem);
                
                serializer.push(bytes.size());
                serializer.push_bytes(bytes);
            }
            
            return std::move(serializer.bytes);
        }
        
        static T from_bytes(std::span<u8> bytes) {
            pop_deserializer deserializer { bytes };
            T result { };
            
            while (!deserializer.bytes.empty()) {
                std::size_t size = deserializer.pop<std::size_t>();
                auto elem_bytes  = deserializer.pop_bytes(size);
                
                result.insert(std::end(result), elem_serializer_t::from_bytes(elem_bytes));
            }
            
            return result;
        }
    };
    
    
    
    
    // Classes with serializable members.
    namespace detail {
        // Cannot be a lambda due to unevaluated context.
        struct is_serializable_member {
            template<typename M> constexpr bool operator()(void) {
                return requires { serializer<M>{}; } || requires { typename M::binary_serializable_tag; };
            }
        };
    }
    
    template <typename T> requires
        std::is_class_v<T> &&
        // There must be a way to construct T.
        std::is_default_constructible_v<T> &&
        // Each member of T must be serializable.
        (meta::data_member_pack<T>::all(detail::is_serializable_member { }))
    struct serializer<T> {
        static std::vector<u8> to_bytes(const T& value) {
            push_serializer serializer { sizeof(T) };
            
            boost::pfr::for_each_field(value, [&] <typename M> (const M& m) {
                std::vector<u8> bytes;
                
                if constexpr (requires { typename M::binary_serializable_tag; }) {
                    bytes = m.to_bytes();
                } else {
                    bytes = serializer<M>::to_bytes(m);
                }
                
                serializer.push(bytes.size());
                serializer.push_bytes(bytes);
            });
            
            return std::move(serializer.bytes);
        }
    
        static T from_bytes(std::span<u8> bytes) {
            pop_deserializer deserializer { bytes };
            T result { };
            
            boost::pfr::for_each_field(result, [&] <typename M> (M& m) {
                std::size_t size  = deserializer.pop<std::size_t>();
                auto member_bytes = deserializer.pop_bytes(size);
                
                if constexpr (requires { typename M::binary_serializable_tag; }) {
                    m = M::from_bytes(member_bytes);
                } else {
                    m = serializer<M>::from_bytes(member_bytes);
                }
            });
            
            return result;
        }
    };
}