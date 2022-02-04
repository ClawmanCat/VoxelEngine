#pragma once

#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/game.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>


namespace demo_game {
    class player : public ve::static_entity {
    public:
        static inline ve::hash_map<ve::instance_id, entt::entity> server_players { };
        static inline entt::entity client_player = entt::null;


        constexpr static inline float player_speed            = 10.0f;
        constexpr static inline float player_look_sensitivity = 1.0f;


        struct player_controller {
            using pfr_blacklist_tag = void;
            ve::instance_id controlled_by;
        };

        struct local_player_tag  { };
        struct remote_player_tag { };


        static void remote_initializer(ve::registry& owner, entt::entity entity, const player_controller& cmp) {
            if (cmp.controlled_by == game::get_client()->get_id()) {
                // This player is us.
                setup_user_input(owner, entity);
                owner.set_component(entity, ve::update_component { &player::update_user_input });
                owner.set_component(entity, local_player_tag { });

                player::client_player = entity;
            } else {
                // This player is controlled by someone else.
                owner.set_component(entity, remote_player_tag { });

                auto texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / "asimov.png");
                auto buffer  = ve::gfx::textured_quad(ve::vec2f { 1.0f }, texture);

                owner.set_component(entity, ve::mesh_component { std::move(buffer) });
                owner.set_component(entity, simple_render_tag { });
            }
        }


        player(ve::registry& owner, ve::instance_id controller) : static_entity(owner) {
            set(player_controller { controller });

            player::server_players[controller] = get_id();
            set(ve::raii_component { [controller] { player::server_players.erase(controller); } });
        }


        static void setup_user_input(ve::registry& owner, entt::entity self);
        static void update_user_input(const ve::invocation_context& ctx, ve::nanoseconds dt);


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::motion_component VE_COMPONENT(motion) = ve::motion_component { };
    };
}