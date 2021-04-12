#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/serialize/serialize.hpp>
#include <VoxelEngine/side/side.hpp>

#include <type_traits>


namespace ve {
    // In what ways can the component be serialized?
    enum class component_serialization_mode : u8 {
        NONE   = 0,
        BINARY = (1 << 0),
        STRING = (1 << 1),
        // Component can be serialized both as bytes and as a string.
        BOTH   = BINARY | STRING,
        // Component can be serialized, but does so outside of the engine serialization system.
        // (Component should provide callbacks for when serialization occurs to do so.)
        MANUAL = (1 << 2)
    };
    
    VE_BITWISE_ENUM(component_serialization_mode);
    
    
    template <
        typename Derived,
        side Side                        = side::SERVER,
        component_serialization_mode CSM = component_serialization_mode::BINARY
    > struct component :
        public std::conditional_t<
            bool(CSM & component_serialization_mode::BINARY),
            binary_serializable<Derived>,
            meta::unique_null_type<"BINARY">
        >,
        public std::conditional_t<
            bool(CSM & component_serialization_mode::STRING),
            string_serializable<Derived>,
            meta::unique_null_type<"STRING">
        >,
        public std::conditional_t<
            bool(CSM & component_serialization_mode::MANUAL),
            externally_serializable<Derived>,
            meta::unique_null_type<"MANUAL">
        >
    {
        using most_derived_t  = Derived;
        using component_tag_t = void;
        
        constexpr static auto side = Side;
        constexpr static auto csm  = CSM;
    };
    
    
    template <typename T> concept component_type = requires { typename T::component_tag_t; };
}