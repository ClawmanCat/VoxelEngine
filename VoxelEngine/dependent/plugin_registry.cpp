#include <VoxelEngine/dependent/plugin_registry.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/expected.hpp>
#include <VoxelEngine/utility/delayed_cast.hpp>
#include <VoxelEngine/utility/vector.hpp>

#include <magic_enum.hpp>


namespace ve {
    plugin_registry& plugin_registry::instance(void) {
        static plugin_registry i;
        return i;
    }
    
    
    void plugin_registry::scan_folder(const fs::path& path) {
        VE_LOG_INFO(cat("Scanning ", path, " for plugins..."));
        std::size_t old_count = unloaded_plugins.size();
        
        for (const auto& entry : fs::recursive_directory_iterator(path)) {
            if (!entry.is_regular_file()) continue;
            if (!is_library_file(entry.path())) continue;
            
            auto preload = try_call(&plugin_registry::preload_plugin, *this, entry.path());
            if (!preload) {
                VE_LOG_DEBUG(cat("Skipping candidate ", entry.path(), ": ", preload.get_error()));
            } else {
                VE_LOG_DEBUG(cat("Added plugin ", entry.path()));
            }
        }
        
        VE_LOG_INFO(cat("Found ", (unloaded_plugins.size() - old_count), " new plugins."));
    }
    
    
    void plugin_registry::unscan_plugin(const std::string& name) {
        VE_LOG_INFO(cat("Removing ", name, " from known plugins list."));
        
        // Forward list, we need the element before the to-be-removed one.
        auto it = plugins.before_begin();
        while (std::next(it)->info->internal_name != name) ++it;
        auto plugin_it = std::next(it);
        
        
        if (plugin_it == plugins.end()) {
            throw std::runtime_error(cat("No such plugin: ", name));
        }
        
        if (plugin_it->state != plugin_data::AVAILABLE) {
            throw std::runtime_error(cat("Cannot unscan loaded plugin: ", name));
        }
        
        auto* plugin = &*plugin_it;
        unloaded_plugins.erase(plugin);
        plugins_by_name.erase(plugin->info->internal_name);
        plugins_by_path.erase(plugin->path);
        plugins_by_id.erase(plugin->id);
        plugins.erase_after(it);
    }
    
    
    void plugin_registry::load_plugin(const std::string& name, bool dynamic) {
        VE_LOG_INFO(cat("Loading plugin ", name));
        
        plugin_data* plugin = nullptr;
        auto assert_cond = [&](u8 error_cases) {
            auto error = check_errors(error_cases, name, action::LOAD, plugin, nullptr);
            if (error) throw std::runtime_error(*error);
        };
        
        
        auto it = plugins_by_name.find(name);
        assert_cond(error_case::PLUGIN_EXISTS);
        
        plugin = it->second;
        assert_cond(error_case::CIRCULAR_DEPENDENCY | error_case::ALREADY_LOADED | error_case::DYNAMIC_ALLOWED);
        
        
        plugin->state = plugin_data::LOADING;
    
        load_dependencies(plugin, dynamic);
        try_call(&library_handle::call<void, actor_id, bool>, plugin->handle, "on_loaded", plugin->id, dynamic);
        
        plugin->state = plugin_data::LOADED;
        
        
        unloaded_plugins.erase(plugin);
        loaded_plugins.emplace(plugin);
    }
    
    
    void plugin_registry::load_all_plugins(bool dynamic) {
        VE_LOG_INFO("Loading all currently known plugins.");
        std::size_t old_count = loaded_plugins.size();
        
        while (!unloaded_plugins.empty()) {
            auto& plugin = *unloaded_plugins.begin();
            load_plugin(plugin->info->internal_name, dynamic);
        }
    
        VE_LOG_INFO(cat("Loaded ", (loaded_plugins.size() - old_count), " new plugins."));
    }
    
    
    void plugin_registry::try_load_all_plugins(bool dynamic) {
        VE_LOG_INFO("Attempting to load all currently known plugins.");
        std::size_t old_count = loaded_plugins.size();
        
        // Can't simply use while (!unloaded_plugins.empty()), since some plugins may error.
        auto plugins = unloaded_plugins | ranges::to<std::vector>;
        
        for (const auto& plugin : plugins) {
            if (plugin->state == plugin_data::LOADED) continue; // Plugin could get loaded indirectly as dependency.
            auto load_result = try_call(&plugin_registry::load_plugin, *this, plugin->info->internal_name, dynamic);
            
            if (!load_result) {
                VE_LOG_ERROR(cat("Failed to load plugin ", plugin->info->internal_name, ": ", load_result.get_error()));
            }
        }
    
        VE_LOG_INFO(cat("Loaded ", (loaded_plugins.size() - old_count), " new plugins."));
    }
    
    
    void plugin_registry::unload_plugin(const std::string& name, bool dynamic) {
        VE_LOG_INFO(cat("Unloading plugin ", name));
    
        plugin_data* plugin = nullptr;
        auto assert_cond = [&](u8 error_cases) {
            auto error = check_errors(error_cases, name, action::UNLOAD, plugin, nullptr);
            if (error) throw std::runtime_error(*error);
        };
        
        
        auto it = plugins_by_name.find(name);
        assert_cond(error_case::PLUGIN_EXISTS);
        
        plugin = it->second;
        assert_cond(error_case::ALREADY_LOADED | error_case::DYNAMIC_ALLOWED);
        
        
        plugin->state = plugin_data::UNLOADING;
    
        unload_dependents(plugin, dynamic);
        try_call(&library_handle::call<void, bool>, plugin->handle, "on_unloaded", dynamic);
    
        plugin->state = plugin_data::AVAILABLE;
        
        
        loaded_plugins.erase(plugin);
        unloaded_plugins.insert(plugin);
    }
    
    
    void plugin_registry::unload_all_plugins(bool dynamic) {
        VE_LOG_INFO("Unloading all currently loaded plugins.");
        std::size_t old_count = loaded_plugins.size();
    
        while (!loaded_plugins.empty()) {
            auto& plugin = *loaded_plugins.begin();
            unload_plugin(plugin->info->internal_name, dynamic);
        }
        
        VE_LOG_INFO(cat("Unloaded ", (old_count - loaded_plugins.size()), " plugins."));
    }
    
    
    void plugin_registry::try_unload_all_plugins(bool dynamic) {
        VE_LOG_INFO("Attempting to unload all currently loaded plugins.");
        std::size_t old_count = loaded_plugins.size();
        
        // Can't simply use while (!loaded_plugins.empty()), since some plugins may error.
        auto plugins = loaded_plugins | ranges::to<std::vector>;
    
        for (const auto& plugin : plugins) {
            if (plugin->state != plugin_data::LOADED) continue; // Plugin could get unloaded indirectly as dependent.
            auto unload_result = try_call(&plugin_registry::unload_plugin, *this, plugin->info->internal_name, dynamic);
        
            if (!unload_result) {
                VE_LOG_ERROR(cat("Failed to unload plugin ", plugin->info->internal_name, ": ", unload_result.get_error()));
            }
        }
    
        VE_LOG_INFO(cat("Unloaded ", (old_count - loaded_plugins.size()), " plugins."));
    }
    
    
    plugin_data* plugin_registry::preload_plugin(const fs::path& path) {
        auto handle = library_handle(path);
        const auto* info = handle.call<const plugin_info*>("get_info");
        
        auto& plugin = plugins.emplace_front(plugin_data {
            .state        = plugin_data::AVAILABLE,
            .handle       = std::move(handle),
            .info         = info,
            .id           = next_actor_id(),
            .path         = fs::absolute(path).string(),
            .dependencies = { },
            .dependents   = { }
        });
        
        unloaded_plugins.emplace(&plugin);
        plugins_by_name.emplace(plugin.info->internal_name, &plugin);
        plugins_by_path.emplace(plugin.path, &plugin);
        plugins_by_id.emplace(plugin.id, &plugin);
        
        return &plugin;
    }
    
    
    void plugin_registry::load_dependencies(plugin_data* data, bool dynamic) {
        for (const auto& dependency : data->info->dependencies) {
            VE_LOG_INFO(cat(
                "Plugin ", data->info->internal_name,
                " has ", (dependency.required ? "a required" : "an optional"),
                " dependency on ", dependency.internal_name, ". ",
                "Attempting to load this plugin first, if its not already loaded."
            ));
            
            plugin_data* plugin = nullptr;
            
            auto assert_dependency = [&](u8 error_cases) {
                auto error = check_errors(error_cases, dependency.internal_name, action::LOAD, plugin, &dependency);
                
                if (error) {
                    if (dependency.required) {
                        throw std::runtime_error(std::move(*error));
                    } else {
                        VE_LOG_ERROR(std::move(*error));
                        return false;
                    }
                } else return true;
            };
            
            
            auto plugin_it = plugins_by_name.find(dependency.internal_name);
            if (!assert_dependency(error_case::PLUGIN_EXISTS)) continue;
            
            plugin = plugin_it->second;
            if (!assert_dependency(error_case::VERSION_ALLOWED)) continue;
            
            
            try {
                if (plugin->state != plugin_data::LOADED) load_plugin(dependency.internal_name, dynamic);
            } catch (std::exception& e) {
                auto msg = cat(
                    "Failed to load ",
                    (dependency.required ? "required" : "optional"),
                    " dependency ", dependency.internal_name, ": ", e.what()
                );
                
                if (dependency.required) {
                    throw std::runtime_error(msg);
                } else {
                    VE_LOG_ERROR(msg);
                    continue;
                }
            }
            
            
            data->dependencies.push_back(plugin);
            plugin->dependents.push_back(data);
        }
    }
    
    
    void plugin_registry::unload_dependents(plugin_data* data, bool dynamic) {
        // Traverse in reverse so we can safely delete elements while iterating.
        for (i64 i = ((i64) data->dependents.size()) - 1; i >= 0; --i) {
            const auto& dependent = data->dependents[i];
            
            VE_LOG_INFO(cat(
                "Plugin ", data->info->internal_name,
                " is required by ", dependent->info->internal_name, ". ",
                "This plugin will be unloaded first."
            ));
            
            
            // Plugin may be unloaded indirectly elsewhere in the dependency chain.
            if (dependent->state == plugin_data::AVAILABLE) continue;
            
            
            try {
                auto error = check_errors(error_case::CIRCULAR_DEPENDENCY, dependent->info->internal_name, action::UNLOAD, dependent);
                if (error) throw std::runtime_error(*error);
                
                unload_plugin(dependent->info->internal_name, dynamic);
                swap_erase_at(data->dependents, (std::size_t) i);
            } catch (std::exception& e) {
                auto msg = cat("Failed to unload dependency ", dependent->info->internal_name, ": ", e.what());
                throw std::runtime_error(std::move(msg));
            }
        }
    }
    
    
    std::optional<std::string> plugin_registry::check_errors(
        u8 error_cases,
        const std::string& name,
        action action,
        const plugin_data* current,
        const plugin_info::dependency* loaded_from
    ) const {
        auto make_error_message = [&](auto&& msg) {
            return cat(
                "Failed to ", to_lowercase(magic_enum::enum_name(action)),
                " plugin ", name, ": ", msg
            );
        };
        
        
        if (error_cases & error_case::CIRCULAR_DEPENDENCY) {
            VE_DEBUG_ASSERT(current, "Parameter may not be null.");
            
            if (current->state & (plugin_data::LOADING | plugin_data::UNLOADING)) {
                return make_error_message("Circular dependency detected.");
            }
        }
        
        
        if (error_cases & error_case::PLUGIN_EXISTS) {
            if (!plugins_by_name.contains(name)) {
                return make_error_message("Plugin does not exist.");
            }
        }
        
        
        if (error_cases & error_case::VERSION_ALLOWED) {
            VE_DEBUG_ASSERT(current && loaded_from, "Parameter may not be null.");
    
            if (!loaded_from->required_version.contains(current->info->version)) {
                return make_error_message(cat(
                    "Version ", current->info->version,
                    " is not within required range ", loaded_from->required_version
                ));
            }
        }
        
        
        if (error_cases & error_case::DYNAMIC_ALLOWED) {
            VE_DEBUG_ASSERT(current, "Parameter may not be null.");
            
            if (action == LOAD && !current->info->allow_dynamic_load) {
                return make_error_message("Plugin may not be loaded dynamically.");
            }
            
            else if (action == UNLOAD && !current->info->allow_dynamic_unload) {
                return make_error_message("Plugin may not be unloaded dynamically.");
            }
        }
        
        
        if (error_cases & error_case::ALREADY_LOADED) {
            VE_DEBUG_ASSERT(current, "Parameter may not be null.");
            
            if (action == LOAD && current->state == plugin_data::LOADED) {
                return make_error_message("Plugin is already loaded.");
            }
    
            else if (action == UNLOAD && current->state == plugin_data::AVAILABLE) {
                return make_error_message("Plugin is already unloaded.");
            }
        }
        
        
        return std::nullopt;
    }
}