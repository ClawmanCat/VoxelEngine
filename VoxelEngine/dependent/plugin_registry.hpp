#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/actor.hpp>
#include <VoxelEngine/dependent/dependent_info.hpp>
#include <VoxelEngine/platform/library_loader/library_loader.hpp>


namespace ve {
    struct plugin_data {
        enum state : u8 { AVAILABLE = (1 << 0), LOADING = (1 << 1), LOADED = (1 << 2), UNLOADING = (1 << 3) } state;
        
        library_handle handle;
        const plugin_info* info;
        actor_id id;
        std::string path;
        
        std::vector<plugin_data*> dependencies, dependents;
    };
    
    
    class plugin_registry {
    public:
        static plugin_registry& instance(void);
        
        // Looks for plugins in the given folder so that they may be loaded later.
        void scan_folder(const fs::path& path);
        // Removes the plugin from the list of scanned plugins. The plugin may not be currently loaded.
        void unscan_plugin(const std::string& name);
        
        // Attempts to load the given plugin(s).
        // Plugin must be already known from a call to scan_folder to be loaded.
        void load_plugin(const std::string& name, bool dynamic = true);
        void load_all_plugins(bool dynamic = true);
        void try_load_all_plugins(bool dynamic = true);
        
        // Attempts to unload the given plugin(s).
        // Plugin must be in the LOADED state before calling this method.
        void unload_plugin(const std::string& name, bool dynamic = true);
        void unload_all_plugins(bool dynamic = true);
        void try_unload_all_plugins(bool dynamic = true);
        
        VE_GET_CREF(plugins);
        VE_GET_CREF(unloaded_plugins);
        VE_GET_CREF(loaded_plugins);
        VE_GET_CREF(plugins_by_name);
        VE_GET_CREF(plugins_by_path);
        VE_GET_CREF(plugins_by_id);
    private:
        std::forward_list<plugin_data> plugins;
        hash_set<plugin_data*> unloaded_plugins, loaded_plugins;
        
        hash_map<std::string, plugin_data*> plugins_by_name;
        hash_map<std::string, plugin_data*> plugins_by_path;
        hash_map<actor_id, plugin_data*> plugins_by_id;
        
        
        plugin_registry(void) = default;
    
        // Makes the plugin at the given path available to be loaded.
        plugin_data* preload_plugin(const fs::path& path);
        // Attempts to load all dependencies of the given plugin.
        void load_dependencies(plugin_data* data, bool dynamic = true);
        // Attempts to unload all plugins that depend on the given plugin.
        void unload_dependents(plugin_data* data, bool dynamic = true);
        
        
        enum error_case : u8 {
            CIRCULAR_DEPENDENCY = (1 << 0), // Requires current
            PLUGIN_EXISTS       = (1 << 1),
            VERSION_ALLOWED     = (1 << 2), // Requires current and loaded_from
            DYNAMIC_ALLOWED     = (1 << 3), // Requires current
            ALREADY_LOADED      = (1 << 4), // Requires current
        };
        
        enum action { LOAD, UNLOAD };
        
        
        // Checks for the given error cases and returns a suitable error string if any of the given errors occurred.
        // Only parameters relevant to the given error cases need be provided,
        // e.g. to check if a plugin is already loaded, its dependency information is not necessary (See error_case).
        std::optional<std::string> check_errors(
            u8 error_cases,
            const std::string& name,
            action action,
            const plugin_data* current = nullptr,
            const plugin_info::dependency* loaded_from = nullptr
        ) const;
    };
}