#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/compiler.hpp>
#include <VoxelEngine/utility/io/paths.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/shader.hpp)
#include VE_GFX_HEADER(shader/shader_helpers.hpp)

#include <shaderc/shaderc.hpp>


namespace ve::gfx {
    using preprocessor_defs_t = vec_map<std::string, std::string>;


    class shader_cache {
    public:
        static shader_cache& instance(void);


        shared<gfxapi::shader> get_shader(std::string_view name) const {
            VE_DEBUG_ASSERT(shaders.contains(name), "Attempt to get non-existent shader ", name);
            return shaders.find(name)->second;
        }


        template <typename Vertex> shared<gfxapi::shader> get_or_load_shader(const std::vector<fs::path>& files, std::string_view name, const preprocessor_defs_t& defs = {}) {
            if (auto it = shaders.find(name); it != shaders.end()) return it->second;
            return load_shader<Vertex>(files, name, defs);
        }


        template <typename Vertex> shared<gfxapi::shader> get_or_load_shader(const fs::path& folder, std::string_view name, const preprocessor_defs_t& defs = {}) {
            if (auto it = shaders.find(name); it != shaders.end()) return it->second;
            return load_shader<Vertex>(folder, name, defs);
        }


        template <typename Vertex> shared<gfxapi::shader> get_or_load_shader(std::string_view name, const preprocessor_defs_t& defs = {}) {
            return get_or_load_shader<Vertex>(io::paths::PATH_SHADERS, name, defs);
        }


        const auto& get_compile_options(void) const { return *compile_options; }
        void set_compile_options(const auto& options) { compile_options = make_unique<shaderc::CompileOptions>(options); }
    private:
        shader_compiler compiler;
        unique<shaderc::CompileOptions> compile_options; // Non-assignable so use pointer.

        hash_map<std::string, shared<gfxapi::shader>> shaders;


        shader_cache(void) : compile_options(make_unique<shaderc::CompileOptions>(gfxapi::shader_helpers::default_compile_options())) {}


        template <typename Vertex> shared<gfxapi::shader> load_shader(const auto& files_or_folder, std::string_view name, const preprocessor_defs_t& defs) {
            shaderc::CompileOptions options = *compile_options;
            for (const auto& [k, v] : defs) options.AddMacroDefinition(k, v);

            auto [it, success] = shaders.emplace(
                std::string { name },
                gfxapi::make_shader<Vertex>(compiler.compile(files_or_folder, name, options))
            );

            return it->second;
        }
    };
}