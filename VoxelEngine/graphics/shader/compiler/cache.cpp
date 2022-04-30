#include <VoxelEngine/graphics/shader/compiler/cache.hpp>
#include <VoxelEngine/graphics/shader/preprocessor/wave_preprocessor.hpp>
#include <VoxelEngine/graphics/shader/preprocessor/binding_generator.hpp>


namespace ve::gfx {
    shader_cache& shader_cache::instance(void) {
        static shader_cache i { };
        return i;
    }


    shader_cache::shader_cfg_hash shader_cache::hash_for_settings(std::string_view name, const shader_compile_settings* settings) {
        std::size_t hash = 0;

        hash_combine(hash, name);
        hash_combine(hash, *settings);

        return hash;
    }


    shader_cache::shader_cache(bool enable_default_preprocessors) : default_compile_options(make_unique<shaderc::CompileOptions>()) {
        gfxapi::compiler_config::prepare_compile_options(*default_compile_options);
        default_settings.compiler_options = default_compile_options.get();

        if (enable_default_preprocessors) setup_default_preprocessors();
    }


    shared<gfxapi::shader> shader_cache::get(std::string_view name, const shader_compile_settings* settings) {
        if (auto it = shaders.find(hash_for_settings(name, settings)); it != shaders.end()) {
            return it->second;
        } else {
            return nullptr;
        }
    }


    void shader_cache::setup_default_preprocessors(void) {
        // Wave preprocessor performs "normal" C-style preprocessing.
        auto preprocessor = make_shared<wave_preprocessor<>>("ve.preprocessor", priority::HIGHEST);
        gfxapi::compiler_config::prepare_default_preprocessor(*preprocessor);

        preprocessor->add_include_path(io::paths::PATH_SHADERS);
        preprocessor->add_context_action([] (auto& wave_ctx, auto& src, auto& ve_ctx) {
            std::string filepath = ve_ctx.template get_object<fs::path>("ve.filepath").remove_filename().string();
            wave_ctx.add_include_path(filepath.c_str());
        });

        compiler.add_preprocessor(std::move(preprocessor));


        // Automatically resolve bindings for UBOs, SSBOs and specialization constants.
        // shaderc also has an option to do this, but it doesn't work with bindings spread across multiple SPIRV blobs.
        auto binding_preprocessor = make_shared<autobinding_preprocessor>("ve.autobinder");
        compiler.add_preprocessor(std::move(binding_preprocessor));
    }
}