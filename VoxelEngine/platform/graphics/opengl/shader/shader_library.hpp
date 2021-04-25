#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_program.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_compiler.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_layout.hpp>


namespace ve::graphics {
    class shader_library {
    public:
        shader_library(void) = delete;
        
        
        // Gets the given shader if it is loaded, or first loads it by combining all shader files
        // in the given directory with the given name.
        static shared<shader_program> get_shader(std::string_view name, const fs::path& folder = io::paths::PATH_SHADERS) {
            return get_shader_impl(name, folder);
        }
    
    
        // Gets the given shader if it is loaded, or first loads it by combining the given shader files.
        static shared<shader_program> get_shader(std::string_view name, const small_vector<fs::path>& files) {
            return get_shader_impl(name, files);
        }
        
        
        // Gets the layout for the given shader, if it is currently loaded.
        static optional<const shader_layout&> get_shader_layout(std::string_view name) {
            auto it = shaders.find(name);
            
            if (it == shaders.end()) return nullopt;
            else return *(it->second.layout);   // Storage has program lifetime, giving out references is safe.
        }
        
    private:
        struct shader_storage {
            shared<shader_program> program;
            unique<shader_layout> layout;
        };
        
        
        static inline hash_map<std::string, shader_storage> shaders { };
        
        
        template <typename... Args>
        static shared<shader_program> get_shader_impl(std::string_view name, Args&&... args) {
            auto it = shaders.find(name);
            if (it != shaders.end()) return it->second.program;
    
            auto shader = shader_compiler::compile_program(std::forward<Args>(args)..., name);
            if (!shader) VE_ASSERT(false, shader.error().what());
    
            return (shaders[name] = shader_storage {
                .program = std::make_shared<shader_program>(std::move(shader->shader)),
                .layout  = std::make_unique<shader_layout>(std::move(shader->layout))
            }).program;
        }
    };
}