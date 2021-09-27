#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/copy.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    // Simple RAII wrapper for Vulkan resources.
    template <typename Resource, typename Destructor = std::function<void(Resource&&)>>
    requires std::is_invocable_v<Destructor, Resource&&>
    struct vk_resource {
        Resource value = VK_NULL_HANDLE;
        Destructor destructor = nullptr;


        vk_resource(void) = default;

        vk_resource(Resource&& resource, Destructor&& destructor = copy(no_op)) :
            value(std::move(resource)), destructor(std::move(destructor))
        {}

        ~vk_resource(void) {
            if (value != VK_NULL_HANDLE) destructor(std::move(value));
        }


        ve_swap_move_only(vk_resource, value, destructor);
        ve_eq_comparable(vk_resource);
        ve_dereference_as(value);


        // Relinquish control of the resource.
        Resource release(void) {
            return std::exchange(value, VK_NULL_HANDLE);
        }

        // Take control of the resource, keeping the current destructor.
        void store(Resource&& resource) {
            if (value != VK_NULL_HANDLE) destructor(std::move(value));
            value = std::move(resource);
        }


        constexpr operator Resource& (void) { return value; }
        constexpr operator const Resource& (void) const { return value; }
    };
}