#include <VEDemoGame/entity/player.hpp>
#include <VEDemoGame/input/key_bindings.hpp>

#include <VoxelEngine/utility/direction.hpp>


namespace demo_game {
    using namespace ve;


    constexpr std::array player_motions {
        std::pair { "move_forwards",  direction::FORWARD  },
        std::pair { "move_backwards", direction::BACKWARD },
        std::pair { "move_left",      direction::LEFT     },
        std::pair { "move_right",     direction::RIGHT    },
        std::pair { "move_up",        direction::UP       },
        std::pair { "move_down",      direction::DOWN     }
    };


    // Tracks user inputs done during the last tick.
    struct user_inputs {
        user_inputs(void) = default;
        ve_rt_move_only(user_inputs);

        ~user_inputs(void) {
            for (const auto& handle : handles) controls.remove_binding(handle);
        }


        vec3f world_motion = vec3f { 0 };
        vec2f mouse_motion = vec2f { 0 };

        std::vector<typename input_binder::binding_handle> handles;
    };


    void player::setup_user_input(ve::registry& owner, entt::entity self) {
        auto& inputs = owner.set_component(self, user_inputs { });

        for (const auto& [name, direction] : player_motions) {
            inputs.handles.push_back(controls.add_binding(
                name,
                [direction = direction, self, &owner] {
                    owner.template get_component<user_inputs>(self).world_motion += direction;
                }
            ));
        }

        inputs.handles.push_back(controls.add_binding(
            "look",
            [self, &owner] (const void* event, std::size_t type_hash) {
                auto& inputs = owner.template get_component<user_inputs>(self);

                auto add_motion = [&] (const auto& args) {
                    // Motion is scaled such that a mouse movement from the center to the side of the window
                    // equals a delta of exactly (pi / 2) * sensitivity.
                    inputs.mouse_motion +=
                        vec2f { get_most_recent_state(args).position - get_previous_state(args).position } /
                        vec2f { args.window->get_canvas_size() } *
                        constants::f32_pi *
                        player_look_sensitivity;
                };

                VE_ASSERT(
                    !input_categories::motion_progress_events_2d::foreach([&] <typename Event> {
                        if (ve::type_hash<Event>() == type_hash) {
                            add_motion(*((const Event*) event));
                            return false;
                        }

                        return true;
                    }),
                    "Unsupported input type for camera look. Input must be of 2d motion progress type."
                );
            }
        ));
    }


    void player::update_user_input(const invocation_context& ctx, nanoseconds dt) {
        constexpr float epsilon = 1e-6f;


        auto [player_transform, player_motion, input_motion]
            = ctx.registry->template get_components<transform_component, motion_component, user_inputs>(ctx.entity);


        // Default motion is zero if there is no user input.
        player_motion.linear_velocity = vec3f { 0 };


        // Account for camera orientation when moving along xz-axes.
        if (glm::any(glm::abs(vec2f { input_motion.world_motion.xz }) > epsilon)) {
            player_motion.linear_velocity.xz =
                glm::normalize(vec2f { (input_motion.world_motion * game::get_camera().get_rotation()).xz }) *
                player_speed;
        }

        // Use global axes when moving along y-axis.
        if (std::abs(input_motion.world_motion.y) > epsilon) {
            player_motion.linear_velocity.y = input_motion.world_motion.y * player_speed;
        }

        // Reset motion for next tick.
        input_motion.world_motion = vec3f { 0 };


        // Clamp the orientation between -pi / 2 and pi / 2 to prevent "somersaulting" of the camera.
        input_motion.mouse_motion.y = std::clamp(
            input_motion.mouse_motion.y,
            -constants::f32_half_pi + epsilon,
            +constants::f32_half_pi - epsilon
        );

        quatf pitch = glm::angleAxis(input_motion.mouse_motion.y, (vec3f) direction::RIGHT);
        quatf yaw   = glm::angleAxis(input_motion.mouse_motion.x, (vec3f) direction::UP);

        // Modify transform directly instead of motion:
        // orientation change should purely depend on mouse motion, not on elapsed time.
        player_transform.rotation = glm::normalize(pitch * yaw);


        // Update the camera.
        game::get_camera().set_position(player_transform.position);
        game::get_camera().set_rotation(player_transform.rotation);
    }
}