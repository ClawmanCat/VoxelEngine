#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>


namespace ve::meta {
    template <typename Container> using container_value_type = std::remove_reference_t<
        decltype(*std::begin(std::declval<Container>()))
    >;


    template <typename Container> using conditionally_const_iterator = std::conditional_t<
        std::is_const_v<Container>,
        typename Container::const_iterator,
        typename Container::iterator
    >;
}