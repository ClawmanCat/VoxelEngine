#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/container/cached.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/direction.hpp>

#include <boost/preprocessor.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include <cmath>


#define VE_IMPL_INVALIDATE(Rep, Data, Elem) Elem.invalidate();

#define VE_GET_SET_CACHED_TF(fn_name, member, tf, ...)              \
[[nodiscard]] auto get_##fn_name(void) const { return member; }     \
                                                                    \
void set_##fn_name(auto&& val) {                                    \
    member = tf(std::forward<decltype(val)>(val));                  \
                                                                    \
    BOOST_PP_SEQ_FOR_EACH(                                          \
        VE_IMPL_INVALIDATE,                                         \
        _,                                                          \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                       \
    );                                                              \
}

#define VE_GET_SET_CACHED(fn_name, member, ...)                     \
VE_GET_SET_CACHED_TF(                                               \
    fn_name,                                                        \
    member,                                                         \
    [](auto&& elem) -> decltype(auto) {                             \
        return std::forward<decltype(elem)>(elem);                  \
    },                                                              \
    __VA_ARGS__                                                     \
)


namespace ve::graphics {
    class perspective_camera {
    public:
        perspective_camera(
            float fov = radians(90.0f),
            float aspect_ratio = 1.0f,
            float near = 0.001f,
            vec3f position = { 0, 0, 0 },
            vec3f rotation = { 0, 0, 0 },
            vec3f scaling  = { 1, 1, 1 }
        ) :
            fov(fov),
            aspect_ratio(aspect_ratio),
            near(near),
            position(position),
            rotation(rotation),
            scaling(scaling),
            forwards(this, &perspective_camera::calculate_forwards),
            right(this, &perspective_camera::calculate_right),
            up(this, &perspective_camera::calculate_up),
            perspective(this, &perspective_camera::calculate_perspective),
            camera(this, &perspective_camera::calculate_camera)
        {}
        
        
        
        void look_at(const vec3f& where) {
            auto delta = glm::normalize(where - position);
    
            set_pitch(asin(delta.y));
            set_yaw(atan2(delta.x, delta.z));
        }
        
        void move(const vec3f& delta) {
            position += delta;
            camera.invalidate();
        }
        
        void rotate(const vec3f& delta) {
            rotation = ve_vec_transform(x, fmodf(x, tau_f))(rotation + delta);
            camera.invalidate();
            forwards.invalidate();
            right.invalidate();
            up.invalidate();
        }
        
        void zoom(const vec3f& delta) {
            scaling *= delta;
            camera.invalidate();
        }
    
    
        void set_aspect_ratio_for_size(const vec2ui& size) {
            aspect_ratio = ((float) size.x) / size.y;
            
            camera.invalidate();
            perspective.invalidate();
        }
        
        
        VE_GET_SET_CACHED(position, position, camera);
        VE_GET_SET_CACHED(scale,    scaling,  camera);
    
        VE_GET_SET_CACHED(x, position[0], camera);
        VE_GET_SET_CACHED(y, position[1], camera);
        VE_GET_SET_CACHED(z, position[2], camera);
        
        VE_GET_SET_CACHED_TF(rotation, rotation, ve_vec_transform(x, fmodf(x, tau_f)), camera, forwards, right, up);
        
        VE_GET_SET_CACHED_TF(pitch, rotation[0], ve_tf_field(x, fmodf(x, tau_f)), camera, forwards, up);
        VE_GET_SET_CACHED_TF(yaw,   rotation[1], ve_tf_field(x, fmodf(x, tau_f)), camera, forwards, right);
        VE_GET_SET_CACHED_TF(roll,  rotation[2], ve_tf_field(x, fmodf(x, tau_f)), camera, right, up);
    
        VE_GET_SET_CACHED(fov,          fov,          camera, perspective);
        VE_GET_SET_CACHED(aspect_ratio, aspect_ratio, camera, perspective);
        VE_GET_SET_CACHED(near,         near,         camera, perspective);
        
        
        [[nodiscard]] vec3f get_forwards (void) const { return +(*forwards); }
        [[nodiscard]] vec3f get_backwards(void) const { return -(*forwards); }
        [[nodiscard]] vec3f get_right    (void) const { return +(*right);    }
        [[nodiscard]] vec3f get_left     (void) const { return -(*right);    }
        [[nodiscard]] vec3f get_up       (void) const { return +(*up);       }
        [[nodiscard]] vec3f get_down     (void) const { return -(*up);       }
        
        [[nodiscard]] mat4f get_matrix(void) const { return camera; }
    private:
        mat4f calculate_perspective(void) const {
            return glm::infinitePerspective(fov, aspect_ratio, near);
        }
    
    
        mat4f calculate_camera(void) const {
            mat4f mat = glm::lookAt(position, position + *forwards, { 0, 1, 0 });
            mat = glm::rotate(mat, get_roll(), *forwards);
        
            return (*perspective) * mat;
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
                glm::normalize(glm::cross(*forwards, (vec3f) direction_vector(direction::UP))),
                get_roll(),
                *forwards
            );
        }
    
    
        vec3f calculate_up(void) const {
            return glm::normalize(glm::cross(*right, *forwards));
        }
        
        
        float fov, aspect_ratio, near;
    
        // Rotation is Yaw, Pitch, Roll.
        vec3f position, rotation, scaling;
        member_cache<vec3f, perspective_camera> forwards, right, up;
        member_cache<mat4f, perspective_camera> perspective, camera;
    };
}