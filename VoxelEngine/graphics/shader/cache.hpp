#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/compiler.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/shader.hpp)

#include <shaderc/shaderc.hpp>


namespace ve::gfx {
    class shader_cache {
    public:
        static shader_cache& instance(void);


        shared<gfxapi::shader> get_shader(std::string_view name) const {
            VE_DEBUG_ASSERT(shaders.contains(name), "Attempt to get non-existent shader ", name);
            return shaders.find(name)->second;
        }


        shared<gfxapi::shader> get_or_load_shader(const std::vector<fs::path>& files, std::string_view name) const {
            if (auto it = shaders.find(name); it != shaders.end()) return it->second;
            return load_shader(files, name);
        }


        shared<gfxapi::shader> get_or_load_shader(const fs::path& folder, std::string_view name) const {
            if (auto it = shaders.find(name); it != shaders.end()) return it->second;
            return load_shader(folder, name);
        }
    private:
        shader_compiler compiler;
        shaderc::CompileOptions compile_options;

        hash_map<std::string, shared<gfxapi::shader>> shaders;


        shader<gfxapi::shader> load_shader(const auto& files_or_folder, std::string_view name) {
            auto [it, success] = shaders.emplace(
                std::string { name },
                gfxapi::make_shader(compiler.compile(files_or_folder, name, compile_options))
            );

            return it->second;
        }

    public:
        VE_GET_SET_CREF(compile_options);
    };
}