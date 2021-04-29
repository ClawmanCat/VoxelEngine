#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/actor.hpp>
#include <VoxelEngine/dependent/plugin.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/platform/library_loader/library_loader.hpp>

#include <string_view>


namespace ve {
    class plugin_loader {
    public:
        enum class plugin_load_type { STATIC, DYNAMIC };
        
        
        static plugin_loader& instance(void);
        
        
        expected<actor_id> load_plugin(const fs::path& path, plugin_load_type load_type);
        void unload_plugin(actor_id id, plugin_load_type load_type);
        
        
        [[nodiscard]] const plugin_info& get_plugin_info(actor_id id) const;
        [[nodiscard]] actor_id actor_from_internal_name(std::string_view name) const;
        [[nodiscard]] library_handle get_native_handle(actor_id id);
        
        
        template <typename Ret, typename... Args>
        Ret invoke_plugin_method(actor_id id, const char* method, Args&&... args) {
            VE_ASSERT(plugins.contains(id), "Attempt to call method from non-existent plugin "s + std::to_string(id));
            
            auto fn = get_library_function<Ret, Args...>(plugins[id].handle, method);
            VE_ASSERT(
                fn,
                "Failed to get method "s + method + " from plugin " + std::to_string(id) + ": " + fn.error()
            );
            
            return (*fn)(std::forward<Args>(args)...);
        }
    private:
        plugin_loader(void) = default;
        
        struct plugin_storage {
            library_handle handle;
            const plugin_info* info;
        };
        
        hash_map<actor_id, plugin_storage> plugins;
        hash_map<std::string_view, actor_id> plugin_names;
    };
}