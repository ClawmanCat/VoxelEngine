#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>

#include <tuple>
#include <type_traits>


namespace ve::meta {
    template <meta::template_instantiation_of<std::tuple>... Tuples>
    using tuple_cat_t = decltype(std::tuple_cat(std::declval<Tuples>()...));
}