#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/graphics/shader/shader_preprocessor.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/stage.hpp)

#include <shaderc/shaderc.hpp>


namespace ve::gfx {
    using spirv_blob = reflect::spirv_blob;


    struct shader_compilation_data {
        vec_map<const gfxapi::shader_stage*, spirv_blob> spirv;
        reflect::shader_reflection reflection;
    };


    class shader_compiler {
    public:
        // Get all files in the given folder whose name without extension is equal to the given name.
        static std::vector<fs::path> get_files_for_shader(const fs::path& folder, std::string_view name);


        shader_compilation_data compile(const std::vector<fs::path>& files, std::string_view name, const shaderc::CompileOptions& options) const;

        shader_compilation_data compile(const fs::path& folder, std::string_view name, const shaderc::CompileOptions& options) const {
            return compile(get_files_for_shader(folder, name), name, options);
        }


        void add_preprocessor(shared<preprocessors::shader_preprocessor> preprocessor);
        void remove_preprocessor(std::string_view name);
        shared<preprocessors::shader_preprocessor> get_preprocessor(std::string_view name);
    private:
        // Sorts from high priority to low priority.
        struct comparator {
            using type = shared<preprocessors::shader_preprocessor>;
            bool operator()(const type& a, const type& b) const { return a->get_priority() > b->get_priority(); }
        };

        tree_multiset<shared<preprocessors::shader_preprocessor>, comparator> preprocessors;


        spirv_blob compile_stage(
            const fs::path& path,
            const std::string& name,
            const io::text_file& data,
            const gfxapi::shader_stage* stage,
            const shaderc::CompileOptions& options
        ) const;
    };
}