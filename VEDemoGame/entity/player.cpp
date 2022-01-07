#include <VEDemoGame/entity/player.hpp>


namespace demo_game {
    void initialize_player_controls(entt::entity player, ve::registry& owner) {
        constexpr float epsilon = 1e-6f;

        constexpr std::array player_motions {
            std::pair { "move_forwards",  ve::direction::FORWARD  },
            std::pair { "move_backwards", ve::direction::BACKWARD },
            std::pair { "move_left",      ve::direction::LEFT     },
            std::pair { "move_right",     ve::direction::RIGHT    },
            std::pair { "move_up",        ve::direction::UP       },
            std::pair { "move_down",      ve::direction::DOWN     }
        };


        auto& input_data = owner.set_component(player, player_input_data { });

        using camera_t = std::remove_cvref_t<decltype(game::get_camera())>;
        owner.set_component(player, ve::camera_controller_component<camera_t> { &game::get_camera() });


        // Handle movement inputs.
        for (const auto& [name, motion] : player_motions) {
            auto handle = controls.bind(
                name,
                [player, &owner, motion = motion] (const ve::binary_input::handler_args& args) {
                    auto& input_data = owner.template get_component<player_input_data>(player);
                    input_data.motion += motion;
                }
            );

            input_data.handles.push_back(handle);
        }


        // Handle look input.
        auto handle = controls.bind(
            "look",
            [player, &owner] (const ve::motion_input::handler_args& args) {
                auto& input_data = owner.template get_component<player_input_data>(player);

                // Motion is scaled to window size so a mouse movement from the center to the bottom of the window
                // equals a movement of exactly (pi / 2) * sensitivity.
                input_data.orientation +=
                    ve::vec2f { args.current.position - args.prev.position } /
                    ve::vec2f { args.window->get_canvas_size() } *
                    ve::constants::f32_pi *
                    player_look_sensitivity;

                // Clamp pitch between -pi / 2 and pi / 2 to remove the ability to somersault the camera.
                // (Note: epsilon is used to prevent looking exactly up / down so we can always define a forward vector.)
                input_data.orientation.y = std::clamp(
                    input_data.orientation.y,
                    -ve::constants::f32_half_pi + epsilon,
                    +ve::constants::f32_half_pi - epsilon
                );
            }
        );

        input_data.handles.push_back(handle);
    }




    void update_player_controls(const ve::invocation_context& ctx, ve::nanoseconds dt) {
        constexpr float epsilon = 1e-6f;

        auto& input_data = ctx.registry->get_component<player_input_data>(ctx.entity);
        auto& motion     = ctx.registry->get_component<ve::motion_component>(ctx.entity);
        auto& transform  = ctx.registry->get_component<ve::transform_component>(ctx.entity);


        motion.linear_velocity = ve::vec3f { 0 };

        if (std::abs(input_data.motion.x) > epsilon || std::abs(input_data.motion.z) > epsilon) {
            // Assure diagonal movement isn't faster than movement along the world axes.
            motion.linear_velocity.xz += glm::normalize(ve::vec2f { (input_data.motion * game::get_camera().get_rotation()).xz }) * player_speed;
        }

        if (std::abs(input_data.motion.y) > epsilon) {
            // Use world up axis for up / down motion.
            motion.linear_velocity.y += input_data.motion.y * player_speed;
        }

        input_data.motion = ve::vec3f { 0 };


        ve::quatf pitch = glm::angleAxis(input_data.orientation.y, (ve::vec3f) ve::direction::RIGHT);
        ve::quatf yaw   = glm::angleAxis(input_data.orientation.x, (ve::vec3f) ve::direction::UP);

        transform.rotation = glm::normalize(pitch * yaw);
    }
}