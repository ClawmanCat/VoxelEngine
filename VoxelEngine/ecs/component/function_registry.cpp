#include <VoxelEngine/ecs/component/function_registry.hpp>
#include <VoxelEngine/dependent/plugin_loader.hpp>
#include <VoxelEngine/utility/io/io.hpp>
#include <VoxelEngine/utility/utility.hpp>


namespace ve {
    function_registry::function_registry(const fs::path& save_folder) {
        fs::path path = save_folder / "functions.dat";
        
        if (fs::exists(path)) {
            auto bytes = *io::read_data(path);
            (*this) = from_bytes(std::span { bytes.begin(), bytes.end() });
            
            VE_LOG_DEBUG("Loaded "s + std::to_string(functions.size()) + " function mappings from disk.");
        }
    }
    
    
    void function_registry::on_actor_created(actor_id id) {
        auto it = functions_by_owner.find(
            plugin_loader::instance().get_plugin_info(id).internal_name
        );
        
        if (it == functions_by_owner.end()) return;
        
        
        library_handle native_handle = plugin_loader::instance().get_native_handle(id);
        
        for (auto& info : it->second | views::indirect) {
            info.fn_ptr = demangle::get_function(
                info.mangled_name.c_str(),
                native_handle
            );
        }
    }
    
    
    void function_registry::on_actor_destroyed(actor_id id) {
        auto it = functions_by_owner.find(
            plugin_loader::instance().get_plugin_info(id).internal_name
        );

        if (it == functions_by_owner.end()) return;


        for (auto& info : it->second | views::indirect) {
            info.fn_ptr = nullptr;
        }
    }
    
    
    std::vector<u8> function_registry::to_bytes(void) const {
        std::vector<u8> result;
        result.reserve(40 * functions.size() + sizeof(u64));
      
        auto store = [&] <typename T> (const T& v, std::size_t size = sizeof(T)) {
            std::size_t current_size = result.size();
            
            result.resize(current_size + size);
            memcpy(&result[current_size], &v, size);
        };

        
        store(next_id);
        
        for (const auto& [mangled_name, owner_name, ptr, id] : functions | views::values) {
            store(mangled_name.size());
            store(mangled_name[0], mangled_name.size());

            store(owner_name.size());
            store(owner_name[0], owner_name.size());
            
            store(id);
        }
        
        
        return result;
    }
    
    
    function_registry function_registry::from_bytes(std::span<u8> bytes) {
        function_registry result;
        
        auto load = [&] <typename T> (T& target, std::size_t size = sizeof(T)) {
            memcpy(&target, &bytes[0], size);
            bytes = bytes.subspan(size);
        };
        
        
        load(result.next_id);
        
        while (!bytes.empty()) {
            function_info info;
            
            u64 mangled_name_len;
            load(mangled_name_len);
            
            info.mangled_name.resize(mangled_name_len);
            load(info.mangled_name[0], mangled_name_len);

            
            u64 owner_name_len;
            load(owner_name_len);

            info.owner_name.resize(owner_name_len);
            load(info.owner_name[0], owner_name_len);
            
            
            load(info.id);
            
            
            if (one_of(info.owner_name, "__engine__", "__game__")) {
                info.fn_ptr = demangle::get_function(info.mangled_name.c_str());
            }
            
            
            auto& stored_info = result.functions.insert({ info.id, std::move(info) }).first->second;
            result.functions_by_ptr.insert({ stored_info.fn_ptr, &stored_info });
            result.functions_by_owner[stored_info.owner_name].push_back(&stored_info);
        }
        
        
        return result;
    }
    
    
    u64 function_registry::register_function_impl(void* fn, const char* mangled_name, actor_id owner) {
        // If the function is already registered, return the existing ID.
        if (auto it = functions_by_ptr.find(fn); it != functions_by_ptr.end()) {
            return it->second->id;
        }
        
        // Otherwise register it.
        u64 id = next_id++;
        
        auto& info = functions.insert({
            id,
            function_info {
                .mangled_name = mangled_name,
                .owner_name   = std::string { get_actor_name(owner) },
                .fn_ptr       = fn,
                .id           = id
            }
        }).first->second;
        
        functions_by_ptr.insert({ info.fn_ptr, &info });
        functions_by_owner[info.owner_name].push_back(&info);
        
        return id;
    }
    
    
    // Gets the function with the given ID. Returns nullptr if the function is from a library that is not currently loaded.
    void* function_registry::get_function(u64 id) {
        auto it = functions.find(id);
        
        // While it is possible to end up in a situation where the function is currently not loaded (fn_ptr = null),
        // it should at least have been registered at some point in the past to even have a valid ID.
        VE_ASSERT(
            it != functions.end(),
            "Attempt to get non-existent function with ID "s + std::to_string(id)
        );
        
        return it->second.fn_ptr;
    }
    
    
    // Gets the name of the library owning a plugin with a given ID.
    // This library may be in an unloaded state, and thus not currently exist in the plugin loader.
    std::string_view function_registry::get_owning_library_name(u64 id) {
        auto it = functions.find(id);
        
        VE_ASSERT(
            it != functions.end(),
            "Attempt to get non-existent function with ID "s + std::to_string(id)
        );
        
        return it->second.owner_name;
    }
    
    
    std::string_view function_registry::get_actor_name(actor_id id) {
        if (id == engine_actor_id) return "__engine__";
        if (id == game_actor_id)   return "__game__";
        return plugin_loader::instance().get_plugin_info(id).internal_name;
    }
}