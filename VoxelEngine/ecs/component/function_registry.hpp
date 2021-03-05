#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/serializable.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/platform/demangle/demangle.hpp>


namespace ve {
    // Maps functions to IDs and vice versa in a serializable way.
    class function_registry :
        public resource_owner<function_registry>,
        public binary_serializable<function_registry>
    {
    public:
        function_registry(void) = default;
        explicit function_registry(const fs::path& save_folder);
        
        ve_move_only(function_registry);
        
        
        void on_actor_created(actor_id id);
        void on_actor_destroyed(actor_id id);
        
        
        std::vector<u8> to_bytes(void) const;
        static function_registry from_bytes(std::span<u8> bytes);
        
        
        template <typename Fn>
        u64 register_function(Fn fn, ve_default_actor(owner)) {
            return register_function_impl((void*) fn, demangle::get_mangled_name(fn), owner);
        }
        
        
        // Gets the function with the given ID. Returns nullptr if the function is from a library that is not currently loaded.
        void* get_function(u64 id);
        
        // Gets the name of the library owning a plugin with a given ID.
        // This library may be in an unloaded state, and thus not currently exist in the plugin loader.
        std::string_view get_owning_library_name(u64 id);
    private:
        u64 next_id = 0;
        
        struct function_info {
            std::string mangled_name;
            std::string owner_name;
            void* fn_ptr = nullptr;
            u64 id;
        };
        
        stable_hash_map<u64, function_info> functions;
        hash_map<void*, function_info*> functions_by_ptr;
        hash_map<std::string, std::vector<function_info*>> functions_by_owner;
    
    
        u64 register_function_impl(void* fn, const char* mangled_name, actor_id owner);
        
        static std::string_view get_actor_name(actor_id id);
    };
}