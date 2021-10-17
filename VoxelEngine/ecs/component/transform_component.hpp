#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/uniform/uniform_convertible.hpp>


namespace ve {
    struct transform_component : public gfx::uniform_convertible<transform_component, mat4f> {
        vec3f position = vec3f { 0 };
        quatf rotation = glm::identity<quatf>();


        // But decompose as a matrix when converting to a uniform.
        std::string get_uniform_name(void) const {
            return "transform";
        }

        mat4f get_uniform_value(void) const {
            return glm::translate(glm::identity<mat4f>(), position) * glm::mat4_cast(rotation);
        }
    };
}