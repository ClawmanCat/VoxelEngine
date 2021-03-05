#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>
#include <VoxelEngine/ecs/component/component.hpp>


namespace ve {
    template <meta::string_arg Name, typename T>
    struct named_component : public T {
        constexpr static std::string_view name = Name.c_string;
        
        
        using T::T;
        
        named_component(const T& o) : T(o) {}
        named_component(T&& o) : T(std::move(o)) {}
    };
}