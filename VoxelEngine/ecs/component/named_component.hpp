#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>
#include <VoxelEngine/ecs/component/component.hpp>


namespace ve {
    template <meta::string_arg Name, typename T>
    struct named_component : public T {
        constexpr static std::string_view name = Name.c_string;
        
        
        named_component(void) = default;
        using T::T;
        
        
        named_component(const named_component&) = default;
        named_component(named_component&&) = default;
        
        named_component& operator=(const named_component&) = default;
        named_component& operator=(named_component&&) = default;
        
        
        named_component(const T& o) : T(o) {}
        named_component(T&& o) : T(std::move(o)) {}
        
        
        named_component& operator=(const T& o) {
            T::operator=(o);
            return *this;
        }
    
        named_component& operator=(T&& o) {
            T::operator=(std::move(o));
            return *this;
        }
    };
}