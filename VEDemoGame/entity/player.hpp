#pragma once

#include <VEDemoGame/game.hpp>
#include <VEDemoGame/input/controls.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/camera/perspective_camera.hpp>
#include <VoxelEngine/utility/direction.hpp>
#include <VoxelEngine/utility/math.hpp>


namespace demo_game {
    class player : public ve::static_entity {
    public:
        constexpr static inline float player_speed            = 10.0f;
        constexpr static inline float player_look_sensitivity = 1.0f;
        constexpr static inline float epsilon                 = 1e-6f;

        using camera_t = ve::gfx::perspective_camera;


        constexpr static inline std::array player_motions {
            std::pair { "move_forwards",  ve::direction::FORWARD  },
            std::pair { "move_backwards", ve::direction::BACKWARD },
            std::pair { "move_left",      ve::direction::LEFT     },
            std::pair { "move_right",     ve::direction::RIGHT    },
            std::pair { "move_up",        ve::direction::UP       },
            std::pair { "move_down",      ve::direction::DOWN     }
        };


        explicit player(ve::registry& registry, camera_t* camera) : ve::static_entity(registry), camera(camera) {
            transform.position.y += 2; // Start above ground.

            camera_controller.camera = camera;
            control_handles.reserve(player_motions.size() + 1);


            // Handle player movement: accumulate all input motions each tick, then move according to camera orientation in update component.
            [&] <std::size_t... Is> (std::index_sequence<Is...>) {
                ([&] <std::size_t I> () {
                    auto handle = controls.bind(player_motions[I].first, [id = get_id(), &owner = get_registry()](const ve::binary_input::handler_args& args) {
                        auto& self = get_self<player>(id, owner);
                        self.input_motion += player_motions[I].second;
                    });

                    control_handles.push_back(handle);
                }.template operator()<Is>(), ...);
            }(std::make_index_sequence<player_motions.size()>());


            // Handle player orientation similarly, but store the orientation itself instead of the delta, so we can clamp pitch.
            auto handle = controls.bind(
                "look",
                [id = get_id(), &owner = get_registry()](const ve::motion_input::handler_args& args) {
                    auto& self = get_self<player>(id, owner);


                    // Scale motion to window size, so a mouse movement from the center to the bottom of the window
                    // equals a delta of exactly (pi / 2) * sensitivity.
                    self.input_orientation +=
                        ve::vec2f { args.current.position - args.prev.position } /
                        ve::vec2f { args.window->get_canvas_size() } *
                        ve::constants::f32_pi *
                        player_look_sensitivity;


                    // Clamp pitch between -pi / 2 and pi / 2 to prevent "somersaulting" of the camera.
                    // Note: epsilon offset is used because if we look straight up / down we cannot define a forward direction for motion.
                    constexpr float half_pi = ve::constants::f32_pi / 2.0f;
                    self.input_orientation.y = std::clamp(self.input_orientation.y, -half_pi + epsilon, half_pi - epsilon);
                }
            );

            control_handles.push_back(handle);
        }


        ~player(void) {
            for (const auto& handle : control_handles) controls.unbind(handle);
        }


        player(player&& other) : ve::static_entity(std::move(other)) {
            std::swap(control_handles, other.control_handles);
            std::swap(camera, other.camera);
        }


        player& operator=(player&& other) {
            static_entity::operator=(std::move(other));

            std::swap(control_handles, other.control_handles);
            std::swap(camera, other.camera);

            return *this;
        }


        void VE_COMPONENT_FN(update)(ve::nanoseconds dt) {
            motion.linear_velocity = ve::vec3f { 0 };

            if (std::abs(input_motion.x) > epsilon || std::abs(input_motion.z) > epsilon) {
                // Assure diagonal movement isn't faster than movement along the world axes.
                motion.linear_velocity.xz += glm::normalize(ve::vec2f { (input_motion * camera->get_rotation()).xz }) * player_speed;
            }

            if (std::abs(input_motion.y) > epsilon) {
                // Use world up axis for up / down motion.
                motion.linear_velocity.y += input_motion.y * player_speed;
            }

            input_motion = ve::vec3f { 0 };


            ve::quatf pitch = glm::angleAxis(input_orientation.y, (ve::vec3f) ve::direction::RIGHT);
            ve::quatf yaw   = glm::angleAxis(input_orientation.x, (ve::vec3f) ve::direction::UP);

            transform.rotation = glm::normalize(pitch * yaw);
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::motion_component VE_COMPONENT(motion) = ve::motion_component { };
        ve::camera_controller_component<camera_t> VE_COMPONENT(camera_controller) = ve::camera_controller_component<camera_t> { };

        // ve::light_component VE_COMPONENT(light_source) = ve::light_component {
        //     .radiance = ve::normalize_color(ve::colors::GOLDENROD) * 50.0f,
        //     .attenuation = 1.25f
        // };
    private:
        std::vector<ve::input_binder::input_handle> control_handles;
        camera_t* camera;

        ve::vec3f input_motion = ve::vec3f { 0 };
        ve::vec2f input_orientation = ve::vec2f { 0 };
    };
}