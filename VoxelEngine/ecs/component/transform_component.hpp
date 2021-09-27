#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/decompose.hpp>


namespace ve {
    struct transform_component {
    public:
        explicit transform_component(const vec3f& position = {}, const quatf& rotation = {}) :
            position(position),
            rotation(rotation),
            transform(calculate_transform(position, rotation))
        {}


        void set_position(const vec3f& position) {
            this->position  = position;
            this->transform = calculate_transform(position, rotation);
        }

        void set_rotation(const quatf& rotation) {
            this->rotation  = rotation;
            this->transform = calculate_transform(position, rotation);
        }

        void move  (const vec3f& delta) { set_position(position + delta); }
        void rotate(const quatf& delta) { set_rotation(rotation * delta); }


        // Decompose as just position + rotation, since the matrix can be inferred from those two alone.
        ve_make_decomposable(transform_component, position, rotation);


        VE_GET_CREF(position);
        VE_GET_CREF(rotation);
        VE_GET_CREF(transform);
    private:
        vec3f position;
        quatf rotation;
        mat4f transform;

        static mat4f calculate_transform(const vec3f& position, const quatf& rotation) {
            return glm::translate((mat4f) rotation, position);
        }
    };
}