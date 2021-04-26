#pragma once

#include <VoxelEngine/core/core.hpp>

#include <functional>


namespace ve::graphics {
    template <typename Resource> struct vk_resource {
        vk_resource(void) = default;
        
        vk_resource(Resource&& res, std::function<void(Resource&&)> destructor = nullptr)
            : value(std::move(res)), destructor(destructor), has_value(true)
        {}
        
        ~vk_resource(void) {
            if (has_value && destructor) destructor(std::move(value));
        }
        
        ve_swap_move_only(vk_resource, value, destructor, has_value);
        
        
        operator Resource&(void) { return value; }
        operator const Resource&(void) const { return value; }
        
        Resource* operator&(void) { return &value; }
        const Resource* operator&(void) const { return &value; }
        
        
        Resource value;
        std::function<void(Resource&&)> destructor;
        bool has_value = false;
    };
}