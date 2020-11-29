#pragma once

#include <VEDemoPlugin/core/core.hpp>

#include <VoxelEngine/dependent/actor_id.hpp>
#include <VoxelEngine/dependent/plugin.hpp>

#include <optional>


namespace demo_plugin {
    class plugin {
    public:
        // Noexcept is required for all plugin_api methods.
        static void on_loaded(ve::actor_id id) noexcept;
        static void on_unloaded(ve::actor_id id) noexcept;
        [[nodiscard]] static const ve::plugin_info* get_info(void) noexcept;
        
        
        [[nodiscard]] static std::optional<ve::actor_id> get_id(void) noexcept {
            return plugin::id;
        }
    private:
        static inline std::optional<ve::actor_id> id = std::nullopt;
    };
}


// Called before plugin load to get information about the plugin.
// This method is required for the plugin to be loaded.
plugin_api const ve::plugin_info* get_plugin_info(void) {
    return demo_plugin::plugin::get_info();
}


// Called when the plugin is loaded / unloaded. You should store the provided actor_id.
// These are both optional but you should probably still implement them.
plugin_api void on_plugin_load(ve::actor_id id) {
    demo_plugin::plugin::on_loaded(id);
}

plugin_api void on_plugin_unload(ve::actor_id id) {
    demo_plugin::plugin::on_unloaded(id);
}