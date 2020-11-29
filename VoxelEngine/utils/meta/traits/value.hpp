#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <auto Val> using value = std::integral_constant<decltype(Val), Val>;
}