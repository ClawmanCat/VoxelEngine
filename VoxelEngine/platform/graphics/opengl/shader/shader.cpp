#include <VoxelEngine/platform/graphics/opengl/shader/shader.hpp>
#include <VoxelEngine/graphics/shader/compiler/cache.hpp>


namespace ve::gfx::opengl {
    bool shader::has_uniform(std::string_view name) const {
        for (const auto& [stage_info, stage] : get_reflection().stages) {
            for (const auto& uniform : stage.uniform_buffers) {
                if (uniform.name == name) return true;
            }
        }

        return false;
    }


    bool shader::has_vertex_attribute(std::string_view name) const {
        for (const auto& uniform : get_reflection().get_input_stage().second.inputs) {
            if (uniform.name == name) return true;
        }

        return false;
    }
}