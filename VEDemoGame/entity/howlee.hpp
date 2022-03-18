#pragma once

#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/entity/world.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/utility/random.hpp>


namespace demo_game {
    class howlee : public ve::static_entity {
    public:
        constexpr static inline unsigned emissive_limit   = 127;
        constexpr static inline float speed               = 3.5f;
        constexpr static inline float terminal_velocity   = 10.0f;
        constexpr static inline float fall_acceleration   = 5.0f;
        constexpr static inline float model_y_offset      = 0.65f;
        constexpr static inline float change_dir_chance   = 0.2f;
        constexpr static inline float build_chance        = 0.025f;
        constexpr static inline float emissive_chance     = 0.05f;
        constexpr static inline float builder_type_chance = 0.25f;


        struct entity_howlee_tag {
            enum { NORMAL, BUILDER, EMISSIVE } type;
        };


        explicit howlee(ve::registry& registry, class world* world) : ve::static_entity(registry), world(std::move(world)) {
            static unsigned emissive_howlees = 0;
            if (ve::cheaprand::random_real() < emissive_chance && emissive_howlees < emissive_limit) {
                tag.type = entity_howlee_tag::EMISSIVE;
                ++emissive_howlees;
            }

            else if (ve::cheaprand::random_real() < builder_type_chance) {
                tag.type = entity_howlee_tag::BUILDER;
            }
        }

        ve_rt_move_only(howlee);


        static void remote_initializer(ve::registry& owner, entt::entity entity, const entity_howlee_tag& cmp) {
            std::string texture_name;
            switch (cmp.type) {
                case entity_howlee_tag::NORMAL:   texture_name = "howlee.png";       break;
                case entity_howlee_tag::BUILDER:  texture_name = "hardhat.png";      break;
                case entity_howlee_tag::EMISSIVE: texture_name = "lightbringer.png"; break;
            }

            auto texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / texture_name);
            auto buffer  = ve::gfx::textured_quad(ve::vec2f { 1.0f }, texture);


            owner.set_component(entity, ve::mesh_component { std::move(buffer) });
            owner.set_component(entity, simple_render_tag { });

            if (cmp.type == entity_howlee_tag::EMISSIVE) {
                owner.set_component(entity, ve::light_component {
                    .radiance    = ve::vec3f { ve::cheaprand::random_real(), ve::cheaprand::random_real(), ve::cheaprand::random_real() } * 2.5f,
                    .attenuation = 2.0f
                });
            }
        }


        void VE_COMPONENT_FN(update)(ve::nanoseconds dt) {
            const float dt_seconds = float(dt.count()) / 1e9f;

            auto tile_at = [&] (const auto& where) {
                return world->voxel.get_space()->get_state(where).tile;
            };


            const auto current_tile = tile_at(transform.position);
            const auto standing_on  = tile_at(transform.position - ve::vec3f { 0, 1, 0 });

            // Distance to ground assuming the tile below us is solid.
            const float distance_to_ground = transform.position.y - std::floor(transform.position.y);


            // If we're inside a tile, move up.
            if (current_tile->is_solid()) {
                motion.linear_velocity.y += fall_acceleration * dt_seconds;
                motion.linear_velocity.y  = std::min(motion.linear_velocity.y, terminal_velocity);
            }

            // If we're not on solid ground, fall down.
            else if (distance_to_ground > model_y_offset || !standing_on->is_solid()) {
                motion.linear_velocity.y -= fall_acceleration * dt_seconds;
                if (motion.linear_velocity.y < -fall_acceleration) motion.linear_velocity.y = -fall_acceleration;
            }

            // Otherwise randomly move around.
            else {
                if (ve::cheaprand::random_real() < change_dir_chance * dt_seconds) {
                    constexpr std::array walk_directions {
                        ve::direction::BACKWARD,
                        ve::direction::FORWARD,
                        ve::direction::LEFT,
                        ve::direction::RIGHT
                    };

                    auto direction = ve::cheaprand::random_element(walk_directions);
                    motion.linear_velocity = ve::vec3f { direction } * speed;
                }


                // Don't walk into tiles.
                auto heading_to = tile_at(transform.position + (motion.linear_velocity * dt_seconds));
                if (heading_to->is_solid()) motion.linear_velocity = ve::vec3f { 0 };


                // And randomly build if this is a builder Howlee.
                if (tag.type == entity_howlee_tag::BUILDER && ve::cheaprand::random_real() < build_chance * dt_seconds) {
                    const auto* tile = ve::cheaprand::random_element(std::array { true, false }) ? tiles::TILE_BRICK : tiles::TILE_EMISSIVE;

                    world->voxel.get_space()->set_state(
                        ve::voxel::tilepos { transform.position },
                        ve::voxel::tile_state { .tile = tile, .meta = 0 }
                    );

                    motion.linear_velocity = ve::vec3f { 0 };
                    transform.position.y += 1.0f;
                }
            }
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::motion_component VE_COMPONENT(motion) = ve::motion_component { };
        entity_howlee_tag VE_COMPONENT(tag) = entity_howlee_tag { .type = entity_howlee_tag::NORMAL };

        class world* world;
    };
}