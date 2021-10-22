#pragma once

#include <VEDemoGame/game.hpp>
#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/entity/world.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/voxel/voxel.hpp>
#include <VoxelEngine/utility/io/paths.hpp>
#include <VoxelEngine/utility/random.hpp>

#include <magic_enum.hpp>


namespace demo_game {
    inline ve::mesh_component make_howlee_mesh(void) {
        auto texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / "howlee.png");
        auto buffer  = ve::gfx::textured_quad(ve::vec2f { 1.0f }, texture);

        return ve::mesh_component { std::move(buffer) };
    }



    class howlee : public ve::static_entity {
    public:
        constexpr static inline float howlee_speed      = 3.5f;
        constexpr static inline float howlee_fall_speed = 10.0f;
        constexpr static inline float howlee_fall_accel = 5.0f;
        constexpr static inline float height_off_ground = 0.65f;
        constexpr static inline float redirect_chance   = 0.2f;
        constexpr static inline float build_chance      = 0.025f;
        constexpr static inline float epsilon           = 1e-6f;

        enum class howlee_type { NORMAL, BUILDER };


        explicit howlee(ve::registry& registry, class world* world) : ve::static_entity(registry), world(world) {
            // TODO: Replace this with instanced rendering once it is supported.
            // Alternatively, if the renderer were to push uniforms into its own storage layer rather than that of the mesh,
            // we could just re-use the buffer for every entity.
            this->mesh = make_howlee_mesh();
            this->type = ve::cheaprand::random_int(0, 1) ? howlee_type::NORMAL : howlee_type::BUILDER;
        }


        void VE_COMPONENT_FN(update)(ve::nanoseconds dt) {
            const float dt_seconds = float(dt.count()) / 1e9f;


            auto tile_at = [&] (const auto& pos) { return world->space.get_state(ve::voxel::tilepos { pos }).tile; };

            auto current_tile = tile_at(transform.position);
            auto standing_on  = tile_at(transform.position - ve::vec3f { 0, 1, 0 });


            // Distance to ground assuming the tile below us is solid.
            const float distance_to_ground = transform.position.y - std::floor(transform.position.y);


            // If we're inside a tile, move up.
            if (current_tile->is_solid()) {
                motion.linear_velocity.y += howlee_fall_speed * dt_seconds;
            }

            // If we're not on solid ground, fall down.
            else if (distance_to_ground > height_off_ground || !standing_on->is_solid()) {
                motion.linear_velocity.y -= howlee_fall_accel * dt_seconds;
                if (motion.linear_velocity.y < -howlee_fall_speed) motion.linear_velocity.y = -howlee_fall_speed;
            }

            else {
                // Otherwise, randomly move around.
                if (ve::cheaprand::random_real() < redirect_chance * dt_seconds) {
                    constexpr std::array walk_directions {
                        ve::direction::BACKWARD,
                        ve::direction::FORWARD,
                        ve::direction::LEFT,
                        ve::direction::RIGHT
                    };

                    auto direction = ve::cheaprand::random_element(walk_directions);
                    motion.linear_velocity = ve::vec3f { direction } * howlee_speed;
                }


                // Don't walk into tiles.
                auto heading_to = tile_at(transform.position + (motion.linear_velocity * dt_seconds));
                if (heading_to->is_solid()) motion.linear_velocity = ve::vec3f { 0 };


                // And randomly build if this is a builder Howlee.
                if (type == howlee_type::BUILDER && ve::cheaprand::random_real() < build_chance * dt_seconds) {
                    world->space.set_state(
                        ve::voxel::tilepos { transform.position },
                        ve::voxel::tile_state { .tile = tiles::TILE_BRICK, .meta = 0 }
                    );

                    motion.linear_velocity = ve::vec3f { 0 };
                    transform.position.y += 1.0f;
                }
            }
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::motion_component VE_COMPONENT(motion) = ve::motion_component { };
        ve::mesh_component VE_COMPONENT(mesh) = ve::mesh_component { };
        render_tag_simple VE_COMPONENT(render_tag) = render_tag_simple { };

    private:
        class world* world;
        howlee_type type;
    };
}