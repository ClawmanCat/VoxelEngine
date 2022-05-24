#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/graphics/shader/compiler/compile_settings.hpp>
#include <VoxelEngine/graphics/shader/preprocessor/shader_preprocessor.hpp>
#include <VoxelEngine/utility/io/paths.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/stage.hpp)


namespace ve::gfx {
    using SPIRV = std::vector<u32>;


    // Passed to per-API implementation to construct shader object.
    // TODO: This object gets regularly copied, but the sources don't change. These should be refactored into some sort of flyweight.
    struct shader_compilation_data {
        vec_map<const gfxapi::shader_stage*, SPIRV> spirv_blobs;
        vec_map<const gfxapi::shader_stage*, std::string> glsl_sources;
        vec_map<const gfxapi::shader_stage*, fs::path> glsl_source_locations;
        reflect::shader_reflection reflection;
        shader_compile_settings settings;
    };


    // Compiles shader sources to SPIRV binaries.
    // The resulting shader_compilation_data can be used with gfxapi::make_shader to construct the actual shader.
    // Note: this class is not thread-safe, even if the set of preprocessors remains unchanged.
    class shader_compiler {
    public:
        using source_list   = vec_map<const gfxapi::shader_stage*, std::string>;
        using spirv_list    = vec_map<const gfxapi::shader_stage*, SPIRV>;
        using location_list = vec_map<const gfxapi::shader_stage*, fs::path>;


        shader_compiler(void);


        // Create a shader from a given list of shader stage source files.
        // 'locations' parameter optionally specifies where the compiler should pretend the sources are located for the purpose of resolving include directives and error messages.
        shader_compilation_data compile(const source_list& sources, std::string_view name, const shader_compile_settings& settings, const location_list& locations = {});
        // Create a shader from all shader sources in the given directory with the given name.
        shader_compilation_data compile(const fs::path& directory, std::string_view name, const shader_compile_settings& settings);
        // Create a shader from the provided shader source files.
        shader_compilation_data compile(const std::vector<fs::path>& files, std::string_view name, const shader_compile_settings& settings);

        void add_preprocessor(shared<shader_preprocessor> preprocessor);
        void remove_preprocessor(std::string_view name);
        shared<shader_preprocessor> get_preprocessor(std::string_view name);
    private:
        struct comparator {
            // Yes, it should be '>', not '<'. Higher priorities should come first.
            bool operator()(const auto& a, const auto& b) const { return a->get_priority() > b->get_priority(); }
        };

        tree_set<shared<shader_preprocessor>, comparator> preprocessors;


        // Note: path may be nullptr if the shader didn't come from a file.
        SPIRV create_blob(
            std::string_view name,
            std::string source,
            const fs::path* path,
            const gfxapi::shader_stage* stage,
            const shader_compile_settings& settings,
            arbitrary_storage& ctx
        );
    };
}