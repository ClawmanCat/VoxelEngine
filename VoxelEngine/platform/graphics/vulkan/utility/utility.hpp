#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/utility/traits/dont_deduce.hpp>

#include <vulkan/vulkan.hpp>


// Loads the function pointer for the given Vulkan function dynamically.
#define VE_VKPTR(instance, name)                                                \
[&]() {                                                                         \
    auto ptr = (PFN_##name) vkGetInstanceProcAddr(instance, #name);             \
    VE_ASSERT(ptr, "Failed to load " #name " method.");                         \
    return ptr;                                                                 \
}()


namespace ve::gfx::vulkan::detail {
    // Given two functions, one which returns the number of items get_count(u32* result),
    // and one which fills an array with a property get_details(u32* count, Property* result),
    // creates a vector of that property.
    template <typename Property>
    inline std::vector<Property> enumerate_vk_property(auto get_count, auto get_details) {
        u32 count = 0;
        get_count(&count);
        if (!count) return {};

        std::vector<Property> properties;
        properties.resize(count);
        get_details(&count, properties.data());

        return properties;
    }


    // Most Vulkan functions can be invoked to get both the count and the properties,
    // so we don't have to pass both separately.
    template <typename Property, typename... Args>
    inline std::vector<Property> enumerate_simple_vk_property(auto fn, Args&&... args) {
        return enumerate_vk_property<Property>(
            [&](auto* count) { fn(std::forward<Args>(args)..., count, nullptr); },
            [&](auto* count, auto* props) { fn(std::forward<Args>(args)..., count, props); }
        );
    }


    // Given some property, a function to fetch the supported values of that property
    // and a set of values which must be present for that property, assert that all required values are present.
    // Properties are compared on the value obtained from calling get_property_id on the property (ID).
    // Deduction of ID is disabled to prevent issues with char[] to char* conversion.
    template <typename Property, typename ID, typename... Args>
    inline void validate_availability(meta::dont_deduce<const std::vector<ID>&> required, auto get_property_id, auto fn, Args&&... args) {
        if (required.empty()) return;


        auto available_properties = enumerate_simple_vk_property<Property>(fn, std::forward<Args>(args)...);
        std::vector<ID> available = available_properties | views::transform(get_property_id) | ranges::to<std::vector>;


        for (const auto& property : required) {
            auto compare = meta::pick<std::is_same_v<ID, const char*>>(strcmp, std::equal_to<ID>{});

            VE_ASSERT(
                ranges::find_if(available, bind_front(compare, property)) != available.end(),
                "Required Vulkan property ", property, " is not available"
            );
        }
    }


    inline vk_resource<VkSemaphore> create_semaphore(VkSemaphoreCreateFlags flags = 0) {
        VkSemaphoreCreateInfo info { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, .flags = flags };

        VkSemaphore semaphore;
        if (vkCreateSemaphore(get_context()->logical_device, &info, nullptr, &semaphore) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan semaphore.");
        }

        return { std::move(semaphore), bind<0, 2>(vkDestroySemaphore, get_context()->logical_device.value, nullptr) };
    }


    inline vk_resource<VkFence> create_fence(VkFenceCreateFlags flags = 0) {
        VkFenceCreateInfo info { .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, .flags = flags };

        VkFence fence;
        if (vkCreateFence(get_context()->logical_device, &info, nullptr, &fence) != VK_SUCCESS) {
            VE_ASSERT(false, "Failed to create Vulkan fence.");
        }

        return { std::move(fence), bind<0, 2>(vkDestroyFence, get_context()->logical_device.value, nullptr) };
    }
}