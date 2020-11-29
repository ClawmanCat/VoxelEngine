#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/io/paths.hpp>
#include <VoxelEngine/utils/meta/immovable.hpp>
#include <VoxelEngine/graphics/render/shader/shader_program.hpp>
#include <VoxelEngine/graphics/render/shader/shader_compiler.hpp>

#include <string>
#include <string_view>
#include <optional>


namespace ve {
    class shader_library {
    public:
        static shader_library& instance(void) {
            static shader_library i;
            return i;
        }
        
        
        template <universal<std::string> Str>
        shared<shader_program> get_or_load(Str&& name) {
            auto it = shader_cache.find(name);
            if (it != shader_cache.end()) return it->second;
            
            auto shader = shader_compiler::compile_program(io::paths::PATH_SHADERS, name);
            if (!shader.has_value()) return nullptr;
            
            it = shader_cache.insert({
                std::forward<Str>(name),
                std::make_shared<shader_program>(shader.value())
            }).first;
            
            return it->second;
        }
    private:
        shader_library(void) = default;
        ve_make_immovable;
        
        hash_map<std::string, shared<shader_program>> shader_cache;
    };
}