#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/compiler/compiler.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/shader.hpp)
#include VE_GFX_HEADER(shader/make_shader.hpp)
#include VE_GFX_HEADER(shader/compiler_config.hpp)

#include <shaderc/shaderc.hpp>


namespace ve::gfx {
    class shader_cache {
    public:
        using shader_cfg_hash = std::size_t;
        using source_list     = shader_compiler::source_list;
        using spirv_list      = shader_compiler::spirv_list;
        using location_list   = shader_compiler::location_list;


        static shader_cache& instance(void);
        static shader_cfg_hash hash_for_settings(std::string_view name, const shader_compile_settings* settings);


        explicit shader_cache(bool enable_default_preprocessors = true);


        // Retrieve the given shader from the cache, or return null if it has not yet been loaded.
        shared<gfxapi::shader> get(std::string_view name, const shader_compile_settings* settings = nullptr);


        // Create a shader from a given list of shader stage source files.
        // 'dirs' parameter optionally specifies where the compiler should pretend the sources are located for the purpose of resolving include directives.
        template <typename Vertex> shared<gfxapi::shader> get_or_load(
            const source_list& sources,
            std::string_view name,
            const shader_compile_settings* settings = nullptr,
            const location_list& dirs = {}
        ) {
            if (!settings) settings = &default_settings;

            auto hash = hash_for_settings(name, settings);
            return get_or_load_impl<Vertex>(hash, sources, name, *settings, dirs);
        }


        // Create a shader from all shader sources in the given directory with the given name.
        template <typename Vertex> shared<gfxapi::shader> get_or_load(
            const fs::path& directory,
            std::string_view name,
            const shader_compile_settings* settings = nullptr
        ) {
            if (!settings) settings = &default_settings;

            auto hash = hash_for_settings(name, settings);
            return get_or_load_impl<Vertex>(hash, directory, name, *settings);
        }


        // Create a shader from the provided shader source files.
        template <typename Vertex> shared<gfxapi::shader> get_or_load(
            const std::vector<fs::path>& files,
            std::string_view name,
            const shader_compile_settings* settings = nullptr
        ) {
            if (!settings) settings = &default_settings;

            auto hash = hash_for_settings(name, settings);
            return get_or_load_impl<Vertex>(hash, files, name, *settings);
        }


        // Create a shader from all shader sources in the default shader directory with the given name.
        template <typename Vertex> shared<gfxapi::shader> get_or_load(
            std::string_view name,
            const shader_compile_settings* settings = nullptr
        ) {
            return get_or_load<Vertex>(io::paths::PATH_SHADERS, name, settings);
        }


        // Create a shader from an existing compiled data object with new compile settings.
        // This can be used to recompile an existing shader.
        shared<gfxapi::shader> get_or_load_compilation_variant(
            const shader_compilation_data& data,
            const shader_compile_settings& settings,
            const ctti::type_id_t& vertex_type,
            const auto& vertex_layout,
            const auto& vertex_layout_reflection
        ) {
            auto hash = hash_for_settings(data.reflection.name, &settings);
            if (auto it = shaders.find(hash); it != shaders.end()) return it->second;


            shader_compilation_data new_data = compiler.compile(
                data.glsl_sources,
                data.reflection.name,
                settings,
                data.glsl_source_locations
            );

            auto [it, success] = shaders.emplace(
                hash,
                gfxapi::make_shader(new_data, this, vertex_type, vertex_layout, vertex_layout_reflection)
            );

            return it->second;
        }


        // Create a shader from an existing compiled data object with new specialization settings.
        // This can be used to respecialize shaders without recompiling them.
        shared<gfxapi::shader> get_or_load_specialization_variant(
            const shader_compilation_data& data,
            const shader_specialize_settings& settings,
            const ctti::type_id_t& vertex_type,
            const auto& vertex_layout,
            const auto& vertex_layout_reflection
        ) {
            auto full_settings = data.settings;
            full_settings.specialization_settings = settings;

            auto hash = hash_for_settings(data.reflection.name, &full_settings);
            if (auto it = shaders.find(hash); it != shaders.end()) return it->second;


            shader_compilation_data new_data = data;
            new_data.settings = std::move(full_settings);

            auto [it, success] = shaders.emplace(
                hash,
                gfxapi::make_shader(new_data, this, vertex_type, vertex_layout, vertex_layout_reflection)
            );

            return it->second;
        }


        VE_GET_MREF(compiler);
        VE_GET_MREF(default_compile_options);
    private:
        shader_compiler compiler;
        hash_map<shader_cfg_hash, shared<gfxapi::shader>> shaders;
        unique<shaderc::CompileOptions> default_compile_options;
        shader_compile_settings default_settings;


        void setup_default_preprocessors(void);


        template <typename Vertex> shared<gfxapi::shader> get_or_load_impl(std::size_t hash, auto&&... args) {
            if (auto it = shaders.find(hash); it != shaders.end()) return it->second;

            auto [it, success] = shaders.emplace(
                hash,
                gfxapi::make_shader<Vertex>(compiler.compile(fwd(args)...), this)
            );

            return it->second;
        }
    };
}