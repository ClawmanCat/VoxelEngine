#include <VoxelEngine/platform/graphics/vulkan/context/context.hpp>
#include <VoxelEngine/platform/graphics/vulkan/context/context_helpers.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>
#include <VoxelEngine/utility/raii.hpp>

#include <magic_enum.hpp>


namespace ve::gfx::vulkan {
    context create_context(SDL_Window* initial_window, const vulkan_settings* settings);


    struct vulkan_global_storage {
        using void_fn = std::function<void(void)>;


        // Assure Vulkan is loaded before any reference to the context is ever made.
        // (Attempting to get the context will initialize the singleton, and this initializer.)
        raii_function<void_fn, void_fn> sdl_vulkan_initializer = {
            bind<0>(SDL_Vulkan_LoadLibrary, nullptr),
            ve_wrap_callable(SDL_Vulkan_UnloadLibrary)
        };

        unique<context> context = nullptr;


        static vulkan_global_storage& instance(void) {
            static vulkan_global_storage i { };
            return i;
        }
    };


    bool is_context_created(void) {
        return vulkan_global_storage::instance().context != nullptr;
    }


    context* get_context(void) {
        return vulkan_global_storage::instance().context.get();
    }


    context* get_or_create_context(SDL_Window* initial_window, const vulkan_settings* settings) {
        if (vulkan_global_storage::instance().context == nullptr) {
            VE_ASSERT(initial_window, "A window must be provided when creating the Vulkan context.");
            vulkan_global_storage::instance().context = make_unique<context>(create_context(initial_window, settings));
        }

        return get_context();
    }


    context create_context(SDL_Window* initial_window, const vulkan_settings* settings) {
        // Explicit initialization of singleton to force loading of Vulkan if it hasn't been already.
        vulkan_global_storage::instance();


        auto instance        = detail::create_instance(initial_window, *settings);
        auto debugger        = settings->enable_debugger ? detail::create_debug_messenger(instance, detail::debug_messenger_callback) : VK_NULL_HANDLE;
        auto physical_device = detail::pick_physical_device(instance, settings->required_device_features);


        const auto required_queue_families = get_global_queue_families(physical_device);

        auto logical_device = detail::create_logical_device(
            physical_device,
            settings->required_device_features,
            required_queue_families,
            settings->device_extensions
        );

        auto queues = create_global_queues(instance, physical_device, logical_device);


        return context {
            .settings        = *settings,
            .instance        = std::move(instance),
            .physical_device = std::move(physical_device),
            .logical_device  = std::move(logical_device),
            .queues          = std::move(queues),
            .debugger        = std::move(debugger)
        };
    }
}