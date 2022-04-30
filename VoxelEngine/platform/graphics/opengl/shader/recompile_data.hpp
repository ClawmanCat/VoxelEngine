#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/compiler/compiler.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/layout.hpp>


namespace ve::gfx { class shader_cache; }


namespace ve::gfx::opengl {
    class shader;


    // Holds the information required to recompile or respecialize shaders with new settings or constants.
    // Note: both *_with_cache methods accept nullptr as their cache argument, in which case the new shader is not cached.
    class shader_recompile_data {
    public:
        // Only shaders that have their source files can be recompiled.
        bool is_recompileable(void) const   { return !original_compile_data.glsl_sources.empty(); }
        // Only SPIR-V shaders can be respecialized (OpenGL normally doesn't support specialization constants).
        bool is_respecializable(void) const { return !original_compile_data.spirv_blobs.empty();  }


        shared<shader> recompile(const shader_compile_settings& new_settings) const { return recompile_with_cache(new_settings, managing_cache); }
        shared<shader> recompile_with_cache(const shader_compile_settings& new_settings, shader_cache* cache) const;

        shared<shader> respecialize(const shader_specialize_settings& new_settings) const { return respecialize_with_cache(new_settings, managing_cache); }
        shared<shader> respecialize_with_cache(const shader_specialize_settings& new_settings, shader_cache* cache) const;


        const reflect::shader_reflection& get_reflection(void) const {
            return original_compile_data.reflection;
        }


        VE_GET_VAL(managing_cache);
        VE_GET_VAL(vertex_layout);
        VE_GET_CREF(original_compile_data);
        VE_GET_CREF(vertex_type);
        VE_GET_CREF(vertex_layout_reflection);
    protected:
        shader_cache* managing_cache;
        shader_compilation_data original_compile_data;
        std::span<const vertex_attribute> vertex_layout;
        ctti::type_id_t vertex_type = meta::null_type_id;
        detail::vertex_layout vertex_layout_reflection;
    };
}