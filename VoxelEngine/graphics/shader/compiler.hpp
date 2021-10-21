#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/graphics/shader/include_handler.hpp>

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
        static std::vector<fs::path> get_files_for_shader(const fs::path& folder, std::string_view name) {
            std::vector<fs::path> files;

            for (const auto& entry : fs::directory_iterator(folder)) {
                if (entry.is_regular_file() && io::get_filename_from_multi_extension(entry.path()) == name) {
                    files.push_back(entry.path());
                }
            }

            return files;
        }


        shader_compilation_data compile(const std::vector<fs::path>& files, std::string_view name, const shaderc::CompileOptions& options) const {
            shader_compilation_data result;

            for (const auto& file : files) {
                auto stage_it = ranges::find_if(gfxapi::shader_stages, equal_on(&gfxapi::shader_stage::file_extension, io::get_full_extension(file)));

                if (stage_it == gfxapi::shader_stages.end()) {
                    VE_LOG_WARN(cat("Skipping file ", file, " while compiling shader ", name, " because it is of unknown type."));
                    continue;
                }

                result.spirv.emplace(
                    &*stage_it,
                    compile_stage(std::string { name }, io::read_text(file), &*stage_it, options)
                );
            }

            result.reflection = reflect::generate_reflection(std::string { name }, result.spirv);
            return result;
        }


        shader_compilation_data compile(const fs::path& folder, std::string_view name, const shaderc::CompileOptions& options) const {
            return compile(get_files_for_shader(folder, name), name, options);
        }

    private:
        spirv_blob compile_stage(const std::string& name, const io::text_file& data, const gfxapi::shader_stage* stage, const shaderc::CompileOptions& options) const {
            shaderc::Compiler compiler;

            std::string file_string = cat_range_with(data, "\n");
            auto result = compiler.CompileGlslToSpv(file_string, stage->shaderc_type, name.c_str(), options);

            if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
                throw std::runtime_error(cat(
                    "Failed to compile ", stage->name,
                    " for shader ", name, ":\n",
                    result.GetErrorMessage()
                ));
            }

            return spirv_blob { result.begin(), result.end() };
        }
    };
}