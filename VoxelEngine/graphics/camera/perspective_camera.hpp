#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/camera/camera.hpp>
#include <VoxelEngine/utility/direction.hpp>
#include <VoxelEngine/utility/cache.hpp>
#include <VoxelEngine/utility/invalidatable_transform.hpp>


#define ve_impl_cam_mutator(name, ...) \
ve_impl_matrix_mutator(name, [](auto& mat) { mat.invalidate(); }, __VA_ARGS__)

#define ve_impl_cam_mutator_tf(name, field, op, ...) \
ve_impl_matrix_mutator_tf(name, [](auto& mat) { mat.invalidate(); }, field, op, __VA_ARGS__)


namespace ve::gfx {
    class perspective_camera : public camera<perspective_camera> {
    public:
        explicit perspective_camera(
            float fov          = glm::radians(90.0f),
            float aspect_ratio = 1.0f,
            float near         = 0.01f,
            vec3f position     = vec3f { 0 },
            quatf rotation     = glm::identity<quatf>(),
            vec3f scaling      = vec3f { 1 }
        ) :
            projection(&perspective_camera::update_projection, this),
            view_projection(&perspective_camera::update_view_projection, this),
            position(position),
            rotation(rotation),
            scaling(scaling),
            fov(fov),
            aspect_ratio(aspect_ratio),
            near_plane(near)
        {}


        ve_impl_cam_mutator(position,     view_projection);
        ve_impl_cam_mutator(scaling,      view_projection);
        ve_impl_cam_mutator(fov,          view_projection, projection);
        ve_impl_cam_mutator(aspect_ratio, view_projection, projection);
        ve_impl_cam_mutator(near_plane,   view_projection, projection);

        ve_impl_cam_mutator_tf(move,   position, +=, view_projection);
        ve_impl_cam_mutator_tf(scale,  scaling,  *=, view_projection);


        void set_rotation(const quatf& quat) {
            rotation = glm::normalize(quat);
            view_projection.invalidate();
        }

        void rotate(const quatf& quat) {
            rotation = glm::normalize(rotation * quat);
            view_projection.invalidate();
        }

        void rotate(const vec3f& axis, float angle) {
            rotate(glm::angleAxis(angle, axis));
        }


        quatf get_rotation(void) const {
            return rotation;
        }

        vec3f get_euler_angles(void) const {
            return glm::eulerAngles(rotation);
        }


        mat4f get_projection_matrix(void) const { return projection; }
        mat4f get_view_projection_matrix(void) const { return view_projection; }


        vec3f forward(void)  const { return vec3f { direction::FORWARD } * rotation;  }
        vec3f backward(void) const { return vec3f { direction::BACKWARD } * rotation; }
        vec3f up(void)       const { return vec3f { direction::UP } * rotation;       }
        vec3f down(void)     const { return vec3f { direction::DOWN } * rotation;     }
        vec3f left(void)     const { return vec3f { direction::LEFT } * rotation;     }
        vec3f right(void)    const { return vec3f { direction::RIGHT } * rotation;    }


        std::string get_uniform_name(void) const {
            return "camera";
        }


        camera_uniform get_uniform_value(void) const {
            return camera_uniform {
                .matrix   = view_projection,
                .position = position,
                .near     = near_plane
            };
        }
    private:
        member_cache<const perspective_camera, mat4f> projection;
        member_cache<const perspective_camera, mat4f> view_projection;

        vec3f position;
        quatf rotation;
        vec3f scaling;

        float fov;
        float aspect_ratio;
        float near_plane;


        mat4f update_projection(void) const {
            return glm::infinitePerspective(fov, aspect_ratio, near_plane);
        }


        mat4f update_view_projection(void) const {
            mat4f view = glm::identity<mat4f>();
            view  = glm::scale(view, scaling);
            view *= glm::mat4_cast(rotation);
            view  = glm::translate(view, -position);

            return (*projection) * view;
        }
    };
}