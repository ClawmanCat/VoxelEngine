#pragma once

#include <VoxelEngine/core/core.hpp>
#include <typeindex>

namespace ve::events {
    struct event {
        virtual ~event(void) = default;
    };
    
    
    template <typename T> concept event_class = std::is_base_of_v<event, std::remove_cvref_t<T>>;
}