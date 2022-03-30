#include <VoxelEngine/graphics/shader/compiler/compiler.hpp>
#include <VoxelEngine/utility/io/file_io.hpp>
#include <VoxelEngine/utility/raii.hpp>


namespace ve::gfx {
    shader_compilation_data shader_compiler::compile(const source_list& sources, std::string_view name, const shader_compile_settings& settings, const location_list& dirs) {
        auto profiler_msg = cat("Compile shader ", name);
        VE_PROFILE_FN(profiler_msg.c_str());


        shader_compilation_data result;

        arbitrary_storage context = settings.preprocessor_context;
        context.store_object<const shader_compile_settings*>("ve.compile_settings", &settings);


        auto get_path = [&] (const auto* stage) {
            return dirs.empty() ? io::paths::PATH_SHADERS : dirs.at(stage);
        };


        for (const auto& [stage, source] : sources) {
            auto path = get_path(stage) / cat(name, stage->file_extension);
            auto blob = create_blob(name, source, path, stage, settings, context);

            result.spirv_blobs.emplace(stage, std::move(blob));
            result.glsl_sources.emplace(stage, source);
            result.glsl_source_locations.emplace(stage, std::move(path));
        }


        result.reflection = reflect::generate_reflection(std::string { name }, result.spirv_blobs);
        result.settings   = settings;

        return result;
    }


    shader_compilation_data shader_compiler::compile(const fs::path& directory, std::string_view name, const shader_compile_settings& settings) {
        std::vector<fs::path> paths;

        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && io::get_filename_from_multi_extension(entry.path()) == name) {
                paths.push_back(entry.path());
            }
        }

        return compile(paths, name, settings);
    }


    shader_compilation_data shader_compiler::compile(const std::vector<fs::path>& files, std::string_view name, const shader_compile_settings& settings) {
        source_list sources;
        location_list locations;

        for (const auto& path : files) {
            auto stage_it = ranges::find_if(gfxapi::shader_stages, equal_on(&gfxapi::shader_stage::file_extension, io::get_full_extension(path)));

            if (stage_it == gfxapi::shader_stages.end()) {
                VE_LOG_WARN(cat("Skipping file ", path, " while compiling shader ", name, " because it is of unknown type."));
                continue;
            }

            sources.emplace(&*stage_it, cat_range_with(io::read_text(path), "\n"));
            locations.emplace(&*stage_it, fs::path { path }.remove_filename());
        }

        return compile(sources, name, settings, locations);
    }


    void shader_compiler::add_preprocessor(shared<shader_preprocessor> preprocessor) {
        preprocessors.emplace(std::move(preprocessor));
    }


    void shader_compiler::remove_preprocessor(std::string_view name) {
        erase_if(preprocessors, [&] (const auto& pp) { return pp->get_name() == name; });
    }


    shared<shader_preprocessor> shader_compiler::get_preprocessor(std::string_view name) {
        auto it = ranges::find_if(preprocessors, [&] (const auto& pp) { return pp->get_name() == name; });
        return (it == preprocessors.end()) ? nullptr : *it;
    }


    SPIRV shader_compiler::create_blob(
        std::string_view name,
        std::string source,
        const fs::path& path,
        const gfxapi::shader_stage* stage,
        const shader_compile_settings& settings,
        arbitrary_storage& ctx
    ) {
        auto profiler_msg = cat("Compile ", stage->name, " for shader ", name);
        VE_PROFILE_FN(profiler_msg.c_str());


        std::string filename = io::get_filename_from_multi_extension(path);
        auto on_error = [&] (std::string_view current_state) {
            fs::path dump_path = io::paths::PATH_LOGS / cat(filename, stage->file_extension);
            io::write_text(dump_path, split(source, "\n"));
            VE_LOG_ERROR(cat("Shader compilation for shader ", name, " failed. Dumping ", current_state, " shader source code to ", dump_path));
        };


        // Preprocess shader.
        auto push_additional_preprocessors = raii_function {
            [&] { preprocessors.insert(settings.additional_preprocessors.begin(), settings.additional_preprocessors.end()); },
            [&] { for (const auto& pp : settings.additional_preprocessors) preprocessors.erase(pp); }
        };

        ctx.store_object<std::string>("ve.filename", filename);
        ctx.store_object<fs::path>("ve.filepath", fs::absolute(path));
        ctx.store_object<const gfxapi::shader_stage*>("ve.shader_stage", stage);

        std::string previous_state;
        std::size_t pass = 0;


        do {
            previous_state = source;

            for (const auto& preprocessor : preprocessors) {
                try {
                    std::invoke(*preprocessor, source, ctx);
                } catch (...) {
                    on_error("partially preprocessed");
                    throw;
                }
            }

            ++pass;
        } while (previous_state != source && pass < settings.preprocessor_recursion_limit);

        if (previous_state != source) {
            on_error("partially_preprocessed");
            throw std::runtime_error(cat("Failed to preprocess ", stage->name, " for shader ", filename, ": maximum recursion depth exceeded."));
        };


        // Compile SPIRV.
        shaderc::Compiler compiler;
        auto result = compiler.CompileGlslToSpv(source, stage->shaderc_type, filename.c_str(), *settings.compiler_options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            on_error("preprocessed");
            throw std::runtime_error(cat("Failed to compile ", stage->name, " for shader ", filename, ":\n", result.GetErrorMessage()));
        }

        return SPIRV { result.begin(), result.end() };
    }
}