#include <VoxelEngine/platform/graphics/opengl/shader/recompile_data.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/make_shader.hpp>
#include <VoxelEngine/graphics/shader/compiler/cache.hpp>


namespace ve::gfx::opengl {
    shared<shader> shader_recompile_data::recompile_with_cache(const shader_compile_settings& new_settings, shader_cache* cache) const {
        VE_ASSERT(is_recompileable(), "Cannot recompile shader that has no associated shader source files.");


        // TODO: This is somewhat wasteful, since we only really need the compiler.
        unique<shader_cache> temp_cache = nullptr;
        if (!cache) {
            temp_cache = make_unique<shader_cache>(false); // At least we don't need to create the preprocessor, since our binary is already compiled.
            cache = temp_cache.get();
        }


        return cache->get_or_load_compilation_variant(
            original_compile_data,
            new_settings,
            vertex_type,
            vertex_layout,
            vertex_layout_reflection
        );
    }


    shared<shader> shader_recompile_data::respecialize_with_cache(const shader_specialize_settings& new_settings, shader_cache* cache) const {
        VE_ASSERT(is_respecializable(), "Cannot respecialize shader that has no associated SPIRV binaries.");


        // TODO: This is somewhat wasteful, since we only really need the compiler.
        unique<shader_cache> temp_cache = nullptr;
        if (!cache) {
            temp_cache = make_unique<shader_cache>(false); // At least we don't need to create the preprocessor, since our binary is already compiled.
            cache = temp_cache.get();
        }


        return cache->get_or_load_specialization_variant(
            original_compile_data,
            new_settings,
            vertex_type,
            vertex_layout,
            vertex_layout_reflection
        );
    }
}