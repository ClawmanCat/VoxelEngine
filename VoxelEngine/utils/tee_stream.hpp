#pragma once

#include <VoxelEngine/core/core.hpp>

#include <boost/iostreams/tee.hpp>
#include <boost/iostreams/stream.hpp>

#include <type_traits>


namespace ve {
    // Create a new stream that streams to all the given streams at once.
    template <typename First, typename... Rest> inline auto make_tee_stream(First& first, Rest&... rest) {
        if constexpr (sizeof...(Rest) > 1) {
            return make_tee_stream(first, make_tee_stream(rest...));
        } else {
            using tee_device = boost::iostreams::tee_device<std::decay_t<First>, std::decay_t<Rest>...>;
            using tee_stream = boost::iostreams::stream<tee_device>;
    
            return tee_stream(tee_device(first, rest...));
        }
    }
    
    
    template <typename... Ts> using tee_stream_t = decltype(make_tee_stream(std::declval<Ts&>()...));
}