#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/serializable.hpp>

#include <type_traits>


namespace ve {
    // On which side should the component be stored?
    enum class component_side : u8 {
        CLIENT = 0b01,
        SERVER = 0b10,
        BOTH   = CLIENT | SERVER
    };
    
    
    // In what ways can the component be serialized?
    enum class component_serialization_mode : u8 {
        NONE   = 0b00,
        BINARY = 0b01,
        STRING = 0b10,
        BOTH   = BINARY | STRING
    };
    
    
    template <
        typename Derived,
        component_side Side              = component_side::SERVER,
        component_serialization_mode CSM = component_serialization_mode::BINARY
    > struct component :
        public std::conditional_t<
            bool(CSM & component_serialization_mode::BINARY),
            binary_serializable<component<Derived, Side, CSM>>,
            meta::unique_null_type<"BINARY">
        >,
        public std::conditional_t<
            bool(CSM & component_serialization_mode::STRING),
            string_serializable<component<Derived, Side, CSM>>,
            meta::unique_null_type<"STRING">
        >
    {
        using most_derived_t  = Derived;
        using component_tag_t = void;
        
        constexpr static auto side = Side;
        constexpr static auto csm  = CSM;
    };
    
    
    template <typename T> concept component_type = requires { typename T::component_tag_t; };
}