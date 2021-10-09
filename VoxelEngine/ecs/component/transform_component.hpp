#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/uniform/uniform_convertible.hpp>
#include <VoxelEngine/utility/decompose.hpp>
#include <VoxelEngine/utility/invalidatable_transform.hpp>


#define ve_impl_tc_mutator(name, ...) \
ve_impl_matrix_mutator(name, [&](auto& mat) { update_transform(); }, __VA_ARGS__)

#define ve_impl_tc_mutator_tf(name, field, op, ...) \
ve_impl_matrix_mutator_tf(name, [&](auto& mat) { update_transform(); }, field, op, __VA_ARGS__)


namespace ve {
    struct transform_component : public gfx::uniform_convertible<transform_component, mat4f> {
    public:
        explicit transform_component(const vec3f& position = {}, const quatf& rotation = {}) :
            position(position),
            rotation(rotation)
        {
            update_transform();
        }


        ve_impl_tc_mutator(position, position, transform);
        ve_impl_tc_mutator(rotation, rotation, transform);
        ve_impl_tc_mutator_tf(move,   position, +=, transform);
        ve_impl_tc_mutator_tf(rotate, rotation, *=, transform);


        // Decompose as just position + rotation, since the matrix can be inferred from those two alone.
        ve_make_decomposable(transform_component, position, rotation);


        // But decompose as a matrix when converting to a uniform.
        std::string get_uniform_name(void) const {
            return "transform";
        }

        mat4f get_uniform_value(void) const {
            return transform;
        }


        VE_GET_CREF(transform);
    private:
        // While it would be possible to use ve::member_cache here for the transform,
        // it is probably preferable to not do so, as it would require extra storage in each transform component.
        vec3f position;
        quatf rotation;
        mat4f transform;

        void update_transform(void) {
            glm::translate(glm::mat4_cast(rotation), position);
        }
    };
}