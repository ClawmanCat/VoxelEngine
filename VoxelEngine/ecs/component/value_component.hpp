#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/serializable.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/ecs/component/named_component.hpp>

#include <type_traits>


namespace ve {
    namespace detail {
        template <typename T> struct trivial_value_component {
            T value;
            VE_WRAP_MEMBER(value);
    
            trivial_value_component(void) = default;
            trivial_value_component(T value) : value(value) {}
        };
        
        
        template <typename T, typename S> struct wrapped_serializer {
            [[nodiscard]] std::vector<u8> to_bytes(void) const {
                return S::to_bytes((T&) *this);
            }
    
            static T from_bytes(std::span<u8> bytes) {
                return S::from_bytes(bytes);
            }
        };
        
        
        // Select a base class for value_component that will serialize it.
        // Will return Pack<S>, where S is the appropriate base class.
        template <typename Self, typename T, typename Serializer, component_serialization_mode CSM>
        constexpr static auto get_value_component_serializer(void) {
            // If the type does not need to be serialized, don't provide a serializer.
            if constexpr (!bool(CSM & component_serialization_mode::BINARY)) {
                return meta::pack<meta::null_type>{};
            }
    
            // If a serializer is provided, use it.
            else if constexpr (!std::is_same_v<Serializer, meta::null_type>) {
                return meta::pack<wrapped_serializer<T, Serializer>>{};
            }
    
            // If T can be serialized, make this class auto-serializable.
            else if constexpr (std::is_trivial_v<T> || requires { typename T::binary_serializable_tag; }) {
                return meta::pack<auto_binary_serializable<Self>>{};
            }
            
            // If the class is marked serializable, but we can't serialize it, throw a compiler error.
            else {
                static_assert(
                    meta::always_false_v<Self>,
                    "This component is marked as binary serializable, but there is no way to serialize it. "
                    "Either make sure the wrapped type is serializable, or provide a serializer for the component."
                );
            }
        }
    
        template <typename Self, typename T, typename Serializer, component_serialization_mode CSM>
        using value_component_serializer_t = typename decltype(get_value_component_serializer<Self, T, Serializer, CSM>())::head;
    }
    
    
    // A component for wrapping simple values.
    template <
        typename T,
        component_side Side              = component_side::SERVER,
        component_serialization_mode CSM = component_serialization_mode::BINARY,
        // If a serializer S is provided, it should implement S::to_bytes(T) and S::from_bytes(T).
        // (Note: this is different from just implementing binary_serializable.)
        typename Serializer              = meta::null_type
    > struct value_component :
        public component<value_component<T>, Side, CSM>,
        // If T is a class, inherit from T, otherwise just store a T inside this object.
        public std::conditional_t<std::is_class_v<T>, T, detail::trivial_value_component<T>>,
        // Inherit from some form of serialization class if this class should be serializable.
        public detail::value_component_serializer_t<value_component<T, Side, CSM, Serializer>, T, Serializer, CSM>
    {
        using value_component_tag = void;
        
        using value_base = std::conditional_t<std::is_class_v<T>, T, detail::trivial_value_component<T>>;
        
        using value_base::value_base;
        value_component(const value_base& o) : value_base(o) {}
        value_component(value_base&& o) : value_base(std::move(o)) {}
    };
    
    
    template <
        meta::string_arg Name,
        typename T,
        component_side Side              = component_side::SERVER,
        component_serialization_mode CSM = component_serialization_mode::BINARY,
        typename Serializer              = meta::null_type
    > using named_value_component = named_component<Name, value_component<T, Side, CSM, Serializer>>;
}