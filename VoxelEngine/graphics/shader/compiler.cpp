#include <VoxelEngine/graphics/shader/compiler.hpp>


namespace ve::gfx {
     std::vector<fs::path> shader_compiler::get_files_for_shader(const fs::path& folder, std::string_view name) {
        std::vector<fs::path> files;

        for (const auto& entry : fs::directory_iterator(folder)) {
            if (entry.is_regular_file() && io::get_filename_from_multi_extension(entry.path()) == name) {
                files.push_back(entry.path());
            }
        }

        return files;
    }




    shader_compilation_data shader_compiler::compile(const std::vector<fs::path>& files, std::string_view name, const shaderc::CompileOptions& options) const {
        shader_compilation_data result;

        for (const auto& file : files) {
            auto stage_it = ranges::find_if(gfxapi::shader_stages, equal_on(&gfxapi::shader_stage::file_extension, io::get_full_extension(file)));

            if (stage_it == gfxapi::shader_stages.end()) {
                VE_LOG_WARN(cat("Skipping file ", file, " while compiling shader ", name, " because it is of unknown type."));
                continue;
            }

            result.spirv.emplace(
                &*stage_it,
                compile_stage(file, std::string { name }, io::read_text(file), &*stage_it, options)
            );
        }

        result.reflection = reflect::generate_reflection(std::string { name }, result.spirv);
        return result;
    }




    void shader_compiler::add_preprocessor(shared<preprocessors::shader_preprocessor> preprocessor) {
        preprocessors.emplace(std::move(preprocessor));
    }




    void shader_compiler::remove_preprocessor(std::string_view name) {
        erase_if(preprocessors, [&] (const auto& pp) { return pp->get_name() == name; });
    }




    shared<preprocessors::shader_preprocessor> shader_compiler::get_preprocessor(std::string_view name) {
        auto it = ranges::find(preprocessors, name, &preprocessors::shader_preprocessor::get_name);
        return (it == preprocessors.end()) ? nullptr : *it;
    }




    spirv_blob shader_compiler::compile_stage(
        const fs::path& path,
        const std::string& name,
        const io::text_file& data,
        const gfxapi::shader_stage* stage,
        const shaderc::CompileOptions& options
    ) const {
        shaderc::Compiler compiler;
        std::string file_string = cat_range_with(data, "\n");


        auto dump_on_error = [&] (std::string_view src_state) {
            fs::path dump_path = io::paths::PATH_LOGS / cat(name, stage->file_extension);
            io::write_text(dump_path, split(file_string, "\n"));
            VE_LOG_ERROR(cat("Dumping ", src_state, " shader source code to ", dump_path));
        };


        // Preprocess shader
        arbitrary_storage context;
        context.store_object<std::string>("ve.filename", name);
        context.store_object<fs::path>("ve.filepath", fs::absolute(path));
        context.store_object<const gfxapi::shader_stage*>("ve.shader_stage", stage);
        context.store_object<const shaderc::CompileOptions*>("ve.compile_options", &options);
        context.store_object<std::vector<std::string>>("ve.completed_stages", std::vector<std::string>{});

        for (const auto& preprocessor : preprocessors | views::indirect) {
            try {
                std::invoke(preprocessor, file_string, context);
            } catch (...) {
                dump_on_error("partially preprocessed");
                throw;
            }

            context.get_object<std::vector<std::string>>("ve.completed_stages").push_back(preprocessor.get_name());
        }


        // Compile shader
        auto result = compiler.CompileGlslToSpv(file_string, stage->shaderc_type, name.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            dump_on_error("preprocessed");
            throw std::runtime_error(cat("Failed to compile ", stage->name, " for shader ", name, ":\n", result.GetErrorMessage()));
        }

        return spirv_blob { result.begin(), result.end() };
    }
}