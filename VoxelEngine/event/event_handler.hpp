#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/threading/threadsafe_counter.hpp>

#include <functional>
#include <utility>


namespace ve::events {
    // Wrap std::function to add an unique ID to identify event handlers with.
    // TODO: Check if this needs to be optimized.
    class event_handler {
    public:
        template <typename... Args> event_handler(Args&&... args) :
            fn(std::forward<Args...>(args...)),
            id(threadsafe_counter<event_handler>::next())
        {}
        
        void operator()(const event& e) const { fn(e); }
        
        [[nodiscard]] u64 get_id(void) const noexcept { return id; }
    private:
        std::function<void(const event&)> fn;
        u64 id;
    };
}