#include <VoxelEngine/dependent/plugin_loader.hpp>
#include <VoxelEngine/dependent/resource_owner_registry.hpp>


namespace ve {
    plugin_loader& plugin_loader::instance(void) {
        static plugin_loader i { };
        return i;
    }
    
    
    expected<actor_id> plugin_loader::load_plugin(const fs::path& path, plugin_load_type load_type) {
        auto handle = load_library(path.string().c_str());
        if (!handle) return make_unexpected(handle.error());
        
        auto info_fn = get_library_function<const plugin_info*>(*handle, "get_plugin_info");
        if (!info_fn) return make_unexpected(info_fn.error());
        
        
        const plugin_info* info_ptr = (*info_fn)();
        
        if (load_type == plugin_load_type::DYNAMIC && !info_ptr->allow_dynamic_load) {
            return make_unexpected("Plugin "s + path.string() + " cannot be loaded dynamically.");
        }
        
        
        actor_id id = next_actor_id();
        
        auto load_fn = get_library_function<void, actor_id>(*handle, "on_plugin_load");
        if (load_fn) (*load_fn)(id);
        
        plugins[id] = plugin_storage {
            .handle = *handle,
            .info   = info_ptr
        };
        
        plugin_names[info_ptr->name] = id;
        
        resource_owner_registry::instance().init_actor_resources(id);
        
        
        VE_LOG_INFO("Plugin "s + info_ptr->name + " was loaded successfully with ID "s + std::to_string(id) + ".");
        return id;
    }
    
    
    void plugin_loader::unload_plugin(actor_id id, plugin_load_type load_type) {
        VE_ASSERT(plugins.contains(id), "Attempt to unload non-existent plugin "s + std::to_string(id));
        auto& storage = (plugins.find(id))->second;
        
        
        if (load_type == plugin_load_type::DYNAMIC && !storage.info->allow_dynamic_unload) {
            VE_ASSERT(false, "Plugin "s + storage.info->name + " cannot be unloaded dynamically.");
        }
        
        
        auto unload_fn = get_library_function<void>(storage.handle, "on_plugin_unload");
        if (unload_fn) (*unload_fn)();
        
        resource_owner_registry::instance().unload_actor_resources(id);
        
        // Must copy name, info struct is about to be destroyed.
        std::string plugin_name = storage.info->name;
        
        unload_library(storage.handle);
        plugins.erase(id);
        plugin_names.erase(plugin_name);
        
        VE_LOG_INFO("Plugin "s + plugin_name + " was unloaded successfully.");
    }
    
    
    [[nodiscard]] const plugin_info& plugin_loader::get_plugin_info(actor_id id) const {
        VE_ASSERT(plugins.contains(id), "Attempt to get info struct for non existent plugin " + std::to_string(id));
        return *(plugins.find(id)->second.info);
    }
    
    
    [[nodiscard]] actor_id plugin_loader::actor_from_internal_name(std::string_view name) const {
        auto it = plugin_names.find(name);
        
        return it == plugin_names.end()
               ? no_actor_id
               : it->second;
    }
    
    
    [[nodiscard]] library_handle plugin_loader::get_native_handle(actor_id id) {
        VE_ASSERT(plugins.contains(id), "Attempt to get handle to non-existent plugin "s + std::to_string(id));
        return plugins[id].handle;
    }
}