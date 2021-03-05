#pragma once

#include <VoxelEngine/core/core.hpp>

#include <type_traits>
#include <string>


namespace ve::meta {
    template <typename Stream, typename Streamed = std::string>
    concept streamable = requires (Stream s, Streamed obj) {
        s << obj;
    };
}