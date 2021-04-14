#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::graphics {
    template <typename Resource> struct vk_resource {
        vk_resource(void) = default;
        
        vk_resource(Resource&& res, Fn<void, Resource&&> destructor)
            : value(std::move(res)), destructor(destructor), has_value(true)
        {}
        
        ~vk_resource(void) {
            if (has_value) destructor(std::move(value));
        }
        
        ve_swap_move_only(vk_resource);
        
        
        operator Resource&(void) { return value; }
        operator const Resouce&(void) const { return value; }
        
        Resource* operator&(void) { return &value; }
        const Resource* operator&(void) const { return &value; }
        
        
        Resource value;
        Fn<void, Resource&&> destructor;
        bool has_value = false;
    };
}