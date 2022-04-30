#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/graphics/shader/compiler/compiler.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/make_shader.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/recompile_data.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/category.hpp>
#include <VoxelEngine/platform/graphics/opengl/context/render_context.hpp>
#include <VoxelEngine/platform/graphics/opengl/vertex/layout.hpp>

#include <gl/glew.h>
#include <ctti/type_id.hpp>


namespace ve::gfx { class shader_cache; }


namespace ve::gfx::opengl {
    class shader : public shader_recompile_data {
    public:
        ~shader(void) { glDeleteShader(id); }
        ve_immovable(shader);


        void bind(render_context& ctx) { glUseProgram(id); }


        bool has_uniform(std::string_view name) const;
        bool has_vertex_attribute(std::string_view name) const;


        VE_GET_VAL(id);
        VE_GET_VAL(category);
    private:
        friend shared<shader> make_shader(const shader_compilation_data&, shader_cache*, const ctti::type_id_t&, std::span<const vertex_attribute>, const detail::vertex_layout&);
        shader(void) = default;

        GLuint id = 0;
        const pipeline_category_t* category;
    };
}