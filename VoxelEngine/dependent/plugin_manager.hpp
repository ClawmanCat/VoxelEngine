#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/plugin.hpp>
#include <VoxelEngine/dependent/actor_id.hpp>
#include <VoxelEngine/utils/immovable.hpp>

#include <VoxelEngine/platform/platform_include.hpp>
#include platform_include(load_library.hpp)

#include <optional>
#include <shared_mutex>
#include <vector>


namespace ve {
    class plugin_manager {
    public:
        static plugin_manager& instance(void) noexcept;
        
        // Attempts to load the plugin from the given path. Returns std::nullopt on failure.
        std::optional<actor_id> load_plugin(const fs::path& path, bool is_dynamic = true);
        
        // Unloads the plugin with the given ID, if it is currently loaded.
        // If this unload is dynamic and the plugin cannot be unloaded dynamically, this method returns false.
        // i.e. returns whether or not the plugin is NOT loaded after calling this method.
        bool unload_plugin(actor_id id, bool is_dynamic = true);
        
        // Unloads all currently loaded plugins.
        // Similarly to unload_plugin, returns true if no more loaded plugins remain after this method has been called.
        bool unload_all_plugins(bool is_dynamic = true);
        
        // Get the ID of every loaded plugin.
        std::vector<actor_id> get_loaded_plugins(void) const noexcept;
        
        // Get a pointer to the info object of the given plugin.
        // If the plugin has been unloaded before locking the weak_ptr, the associated info object will be deleted as well.
        // If the plugin does not exist, the returned pointer will be nullptr.
        weak<plugin_info> get_plugin_info(actor_id id) const noexcept;
    private:
        plugin_manager(void) = default;
        ve_make_immovable;
    
        struct plugin_storage {
            Fn<const plugin_info*> get_info_fn;
            std::optional<Fn<void, actor_id>> on_loaded_fn;
            std::optional<Fn<void, actor_id>> on_unloaded_fn;
            
            library_handle handle;
            shared<plugin_info> info;
            actor_id id;
        };
    
        hash_map<actor_id, plugin_storage> storage;
        mutable std::shared_mutex mtx;
    
    
        // Unloads the given plugin but does not erase it from storage.
        // This method does not lock the mutex. Caller should make sure to do so.
        bool unload_plugin_impl(typename decltype(storage)::iterator it, bool is_dynamic);
    };
}