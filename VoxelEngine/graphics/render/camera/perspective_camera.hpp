#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/cached.hpp>
#include <VoxelEngine/utils/direction.hpp>
#include <VoxelEngine/utils/math_utils.hpp>
#include <VoxelEngine/utils/functional.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <functional>
#include <cmath>


#define VE_IMPL_INVALIDATE(Rep, Data, Elem) Elem.invalidate();

#define VE_IMPL_INVALIDATE_ALL(...)                                             \
BOOST_PP_SEQ_FOR_EACH(                                                          \
    VE_IMPL_INVALIDATE,                                                         \
    _,                                                                          \
    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                       \
)

#define VE_IMPL_GET_SET_INVALIDATE_NVT(name, value, transform, ...)             \
[[nodiscard]] auto get_##name(void) const {                                     \
    return value;                                                               \
}                                                                               \
                                                                                \
void set_##name(auto&& val) {                                                   \
    VE_IMPL_INVALIDATE_ALL(__VA_ARGS__)                                         \
                                                                                \
    this->value = transform(std::forward<decltype(val)>(val));                  \
}

#define VE_IMPL_GET_SET_INVALIDATE_NV(name, value, ...) VE_IMPL_GET_SET_INVALIDATE_NVT(name, value, ve::identity, __VA_ARGS__)
#define VE_IMPL_GET_SET_INVALIDATE_V(value, ...) VE_IMPL_GET_SET_INVALIDATE_NV(value, value, __VA_ARGS__)


namespace ve {
    class perspective_camera {
    public:
        perspective_camera(float fov = radians(90.0f), float aspect_ratio = (16.0f / 9.0f), float near = 0.001f) :
            fov(fov),
            aspect_ratio(aspect_ratio),
            near(near),
            position({ 0, 0, 0 }),
            rotation({ 0, 0, 0 }),
            scaling ({ 0, 0, 0 }),
            forwards(this, &perspective_camera::calculate_forwards),
            right(this, &perspective_camera::calculate_right),
            up(this, &perspective_camera::calculate_up),
            perspective_matrix(this, &perspective_camera::calculate_perspective_matrix),
            camera_matrix(this, &perspective_camera::calculate_camera_matrix)
        {}
        
        
        void look_at(const vec3f& where) {
            auto delta = glm::normalize(where - position);
            
            set_pitch(asin(delta.y));
            set_yaw(atan2(delta.x, delta.z));
        }
    
    
        void move  (const vec3f& delta) { position += delta; VE_IMPL_INVALIDATE_ALL(camera_matrix) }
        void scale (const vec3f& delta) { scaling  *= delta; VE_IMPL_INVALIDATE_ALL(camera_matrix) }
        
        void rotate(const vec3f& delta) {
            rotation += ve_vec_transform(x, fmodf(x, 2 * pi))(delta);
            VE_IMPL_INVALIDATE_ALL(camera_matrix, forwards, right, up)
        }
    
        
        VE_IMPL_GET_SET_INVALIDATE_V(position, camera_matrix);
        VE_IMPL_GET_SET_INVALIDATE_V(scaling,  camera_matrix);
    
        VE_IMPL_GET_SET_INVALIDATE_NVT(
            rotation,
            rotation,
            ve_vec_transform(x, fmodf(x, 2 * pi)),
            camera_matrix, forwards, right, up
        );
    
        
        VE_IMPL_GET_SET_INVALIDATE_NVT(pitch, rotation[0], ve_transform(x, fmodf(x, 2 * pi)), camera_matrix, forwards, up);
        VE_IMPL_GET_SET_INVALIDATE_NVT(yaw,   rotation[1], ve_transform(x, fmodf(x, 2 * pi)), camera_matrix, forwards, right);
        VE_IMPL_GET_SET_INVALIDATE_NVT(roll,  rotation[2], ve_transform(x, fmodf(x, 2 * pi)), camera_matrix, right, up);
        
        VE_IMPL_GET_SET_INVALIDATE_NV(x, position[0], camera_matrix);
        VE_IMPL_GET_SET_INVALIDATE_NV(y, position[1], camera_matrix);
        VE_IMPL_GET_SET_INVALIDATE_NV(z, position[2], camera_matrix);
        
        
        VE_IMPL_GET_SET_INVALIDATE_V(fov,          camera_matrix);
        VE_IMPL_GET_SET_INVALIDATE_V(aspect_ratio, camera_matrix);
        VE_IMPL_GET_SET_INVALIDATE_V(near,         camera_matrix);
        
        
        [[nodiscard]] vec3f get_forwards(void) const { return forwards;      }
        [[nodiscard]] vec3f get_right   (void) const { return right;         }
        [[nodiscard]] vec3f get_up      (void) const { return up;            }
        [[nodiscard]] mat4f get_matrix  (void) const { return camera_matrix; }
    private:
        mat4f calculate_perspective_matrix(void) const {
            //return glm::infinitePerspective(fov, aspect_ratio, near);
            return glm::perspective(fov, aspect_ratio, 0.001f, 100.0f);
        }
    
    
        mat4f calculate_camera_matrix(void) const {
            mat4f mat = glm::lookAt(position, position + *forwards, { 0, 1, 0 });
            // mat = glm::rotate(mat, get_roll(), *forwards);
            
            return (*perspective_matrix) * mat;
        }
    
    
        vec3f calculate_forwards(void) const {
            return glm::normalize(vec3f {
                cos(get_pitch()) * sin(get_yaw()),
                sin(get_pitch()),
                cos(get_pitch()) * cos(get_yaw())
            });
        }
    
    
        vec3f calculate_right(void) const {
            return glm::rotate(
                glm::normalize(glm::cross((vec3f) directions::UP, *forwards)),
                get_roll(),
                *forwards
            );
        }
    
    
        vec3f calculate_up(void) const {
            return glm::normalize(glm::cross(*right, *forwards));
        }
    
    
        float fov, aspect_ratio, near;
        
        // Rotation in pitch, yaw, roll format. (radians)
        vec3f position, rotation, scaling;
        member_cache<vec3f, perspective_camera> forwards, right, up;
        member_cache<mat4f, perspective_camera> perspective_matrix, camera_matrix;
    };
}