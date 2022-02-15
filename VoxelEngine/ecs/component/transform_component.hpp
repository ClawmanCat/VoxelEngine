#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/uniform/uniform_convertible.hpp>
#include <VoxelEngine/utility/decompose.hpp>


namespace ve {
    struct transform_component : public gfx::uniform_convertible<transform_component, mat4f> {
        vec3f position = vec3f { 0 };
        vec3f scale    = vec3f { 1 };
        quatf rotation = glm::identity<quatf>();


        std::string get_uniform_name(void) const {
            return "transform";
        }

        mat4f get_uniform_value(void) const {
            return glm::translate(glm::identity<mat4f>(), position) * glm::mat4_cast(rotation) * glm::scale(glm::identity<mat4f>(), scale);
        }

        producer_t get_uniform_combine_function(void) const {
            return gfx::combine_functions::multiply;
        }


        ve_make_decomposable(transform_component, position, scale, rotation);
    };
}