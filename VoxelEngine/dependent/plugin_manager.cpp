#include <VoxelEngine/dependent/plugin_manager.hpp>
#include <VoxelEngine/dependent/resource_owner_registry.hpp>
#include <VoxelEngine/utils/logger.hpp>
#include <VoxelEngine/utils/functional.hpp>
#include <VoxelEngine/utils/char_convert.hpp>
#include <VoxelEngine/utils/erase_if_it.hpp>
#include <VoxelEngine/utils/meta/bind.hpp>
#include <VoxelEngine/threading/shared_lock_guard.hpp>

#include <filesystem>
#include <algorithm>


namespace ve {
    plugin_manager& plugin_manager::instance(void) noexcept {
        static plugin_manager i { };
        return i;
    }
    
    
    std::optional<actor_id> plugin_manager::load_plugin(const fs::path& path, bool is_dynamic) {
        auto warn_and_return = [&](const auto& msg) { VE_LOG_WARN("Failed to load plugin: "s + msg); return std::nullopt; };
        
        std::lock_guard lock { mtx };
        VE_LOG_DEBUG("Attempting to load plugin "s + path.string());
        
        
        library_handle handle = nullptr;
        
        // This template wrapper is required to prevent the compiler from evaluating the false branch.
        // (path::value_type is os-dependent, but since this method is not a template it is evaluated directly.)
        [&] <typename Path> (const Path& p) {
            if constexpr (std::is_same_v<typename Path::value_type, wchar_t>) {
                handle = load_library(wchars_to_string(p.c_str()).c_str());
            } else {
                handle = load_library(p.c_str());
            }
        }(path);
        
        
        if (!handle) return warn_and_return("Library handle could not be created.");
        
        auto info_fn = get_library_fn<const plugin_info*>(handle, "get_plugin_info");
        if (!info_fn) return warn_and_return("Plugin is missing required method get_plugin_info.");
        if (!info_fn()->allow_dynamic_load && is_dynamic) return warn_and_return("Plugin cannot be loaded dynamically.");
        
        
        using plugin_fn_t = std::optional<Fn<void, actor_id>>;
        plugin_storage data_storage {
            .get_info_fn    = info_fn,
            .on_loaded_fn   = get_library_fn<void, actor_id>(handle, "on_plugin_load") ?: (plugin_fn_t) std::nullopt,
            .on_unloaded_fn = get_library_fn<void, actor_id>(handle, "on_plugin_unload") ?: (plugin_fn_t) std::nullopt,
            .handle         = handle,
            .info           = std::make_shared<plugin_info>(*info_fn()),
            .id             = next_actor_id()
        };
        
        
        actor_id id = data_storage.id;
        
        data_storage.on_loaded_fn.value_or(no_op)(id);
        auto [it, success] = storage.insert({ id, std::move(data_storage) });
        
        VE_LOG_INFO("Successfully loaded plugin "s + it->second.info->name);
        
        return id;
    }
    
    
    bool plugin_manager::unload_plugin(actor_id id, bool is_dynamic) {
        auto warn_and_return = [&](const auto& msg, bool code) { VE_LOG_WARN("Failed to unload plugin: "s + msg); return code; };
        
        std::lock_guard lock { mtx };
        VE_LOG_DEBUG("Attempting to unload plugin with ID "s + std::to_string(id));
        
        auto it = storage.find(id);
        if (it == storage.end()) return warn_and_return("No such plugin exists", true);
        
        bool was_unloaded = unload_plugin_impl(it, is_dynamic);
        if (was_unloaded) storage.erase(it);
        
        return was_unloaded;
    }
    
    
    bool plugin_manager::unload_all_plugins(bool is_dynamic) {
        std::lock_guard lock { mtx };
        
        erase_if_it(
            storage,
            meta::bind_back(meta::bind_front(&plugin_manager::unload_plugin_impl, this), is_dynamic)
        );
        
        return storage.empty();
    }
    
    
    bool plugin_manager::unload_plugin_impl(typename decltype(storage)::iterator it, bool is_dynamic) {
        auto warn_and_return = [&](const auto& msg, bool code) { VE_LOG_WARN("Failed to unload plugin: "s + msg); return code; };
    
    
        if (it == storage.end()) return warn_and_return("No such plugin exists", true);
    
        auto& [plugin_id, data_storage] = *it;
    
        if (!data_storage.info->allow_dynamic_unload && is_dynamic) {
            return warn_and_return("Plugin cannot be unloaded dynamically", false);
        }
    
        data_storage.on_unloaded_fn.value_or(no_op)(plugin_id);
        resource_owner_registry::instance().on_actor_destroyed(plugin_id);
        
        unload_library(data_storage.handle);
        
        VE_LOG_INFO("Successfully unloaded plugin "s + data_storage.info->name);
        return true;
    }
    
    
    std::vector<actor_id> plugin_manager::get_loaded_plugins(void) const noexcept {
        shared_lock_guard lock { mtx };
        
        std::vector<actor_id> result;
        result.reserve(storage.size());
        
        std::transform(
            storage.begin(),
            storage.end(),
            std::back_inserter(result),
            [](const auto& kv) { return kv.first; }
        );
        
        return result;
    }
    
    
    weak<plugin_info> plugin_manager::get_plugin_info(actor_id id) const noexcept {
        shared_lock_guard lock { mtx };
        
        auto it = storage.find(id);
        
        if (it == storage.end()) return weak<plugin_info>();
        else return it->second.info;
    }
}