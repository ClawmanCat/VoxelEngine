#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/shader_compiler.hpp>
#include <VoxelEngine/platform/platform_include.hpp>
#include VE_GRAPHICS_INCLUDE(shader/shader_program.hpp)


namespace ve::graphics {
    class shader_library {
    public:
        static shader_library& instance(void);
        
        
        // Gets the given shader if it is loaded, or first loads it by combining all shader files
        // in the given directory with the given name.
        shared<shader_program> get_shader(std::string_view name, const fs::path& folder = io::paths::PATH_SHADERS) {
            return get_shader_impl(name, folder);
        }
    
    
        // Gets the given shader if it is loaded, or first loads it by combining the given shader files.
        shared<shader_program> get_shader(std::string_view name, const small_vector<fs::path>& files) {
            return get_shader_impl(name, files);
        }
    private:
        shader_compiler compiler;
        hash_map<std::string, shared<shader_program>> shaders;
        
        
        shared<shader_program> get_shader_impl(std::string_view name, auto&&... args) {
            {
                auto it = shaders.find(name);
                if (it != shaders.end()) return it->second;
            }
            
            std::string name_str = std::string(name);
            auto data = compiler.compile_shader(name_str, std::forward<decltype(args)>(args)...);
            
            auto [it, success] = shaders.insert({
                std::move(name_str),
                std::make_unique<shader_program>(data)
            });
            
            return it->second;
        }
    };
}