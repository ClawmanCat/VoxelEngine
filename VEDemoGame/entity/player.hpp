#pragma once

#include <VEDemoGame/game.hpp>
#include <VEDemoGame/input.hpp>
#include <VEDemoGame/component/render_tags.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/utility/color.hpp>


namespace demo_game {
    constexpr float player_speed            = 10.0f;
    constexpr float player_look_sensitivity = 1.0f;


    struct player_input_data {
        ve::vec3f motion { 0 };
        ve::vec2f orientation { 0 };
        std::vector<ve::input_binder::input_handle> handles { };

        player_input_data(void) = default;
        ve_rt_swap_move_only(player_input_data, motion, orientation, handles);

        ~player_input_data(void) {
            for (const auto& handle : handles) controls.unbind(handle);
        }
    };


    extern void initialize_player_controls(entt::entity player, ve::registry& owner);
    extern void update_player_controls(const ve::invocation_context& ctx, ve::nanoseconds dt);


    class player : public ve::static_entity {
    public:
        struct player_id {
            using pfr_blacklist_tag = void; // Boost uuid breaks PFR.
            ve::instance_id player;
        };

        struct local_player_tag  { using non_removable_tag = void; };
        struct remote_player_tag { using non_removable_tag = void; };


        static ve::mesh_component make_mesh(void) {
            auto texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / "asimov.png");
            auto buffer  = ve::gfx::textured_quad(ve::vec2f { 1.0f }, texture);

            return ve::mesh_component { std::move(buffer) };
        }


        static void remote_initializer(entt::entity entity, ve::registry& owner, const player_id& data) {
            if (data.player == static_cast<ve::client&>(owner).get_id()) {
                // This entity is the player associated with this client, add a controller.
                initialize_player_controls(entity, owner);
                owner.set_component(entity, ve::update_component { &update_player_controls });
                owner.set_component(entity, local_player_tag { });
            } else {
                // This entity is the player associated with another client, add a mesh.
                owner.set_component(entity, make_mesh());
                owner.set_component(entity, remote_player_tag { });
                owner.set_component(entity, simple_render_tag { });
            }
        }


        player(ve::registry& owner, ve::instance_id id) : ve::static_entity(owner) {
            set(player_id { id });
            set(ve::remote_init_for(player_id { id }));
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::motion_component VE_COMPONENT(motion) = ve::motion_component { };

        ve::light_component VE_COMPONENT(light_source) = ve::light_component {
            .radiance    = 50.0f * ve::normalize_color(ve::colors::GOLDENROD),
            .attenuation = 1.50f
        };
    };
}