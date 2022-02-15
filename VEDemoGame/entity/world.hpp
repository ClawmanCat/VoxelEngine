#pragma once

#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/tile/tiles.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/voxel/voxel.hpp>
#include <VoxelEngine/utility/noise.hpp>


namespace demo_game {
    class world : public ve::static_entity {
    public:
        static inline ve::voxel::world_layers get_world_layers(void) {
            ve::voxel::world_layers result;

            result.set_sky(ve::voxel::tiles::TILE_AIR);
            result.add_layer(-1, tiles::TILE_STONE);
            result.add_layer(0,  tiles::TILE_GRASS);

            return result;
        }

        static inline ve::unique<ve::voxel::simple_noise_generator> get_world_generator(void) {
            return ve::make_unique<ve::voxel::simple_noise_generator>(
                ve::voxel::simple_noise_generator::arguments {
                    .heightmap = ve::noise::from_file(ve::io::paths::PATH_NOISE / "mountains_valleys_1.noise"),
                    .layers    = get_world_layers()
                }
            );
        }


        explicit world(ve::registry& registry) : ve::static_entity(registry) {
            auto load_where  = ve::voxel::tilepos { 0 };

            #ifdef VE_DEBUG
                auto load_radius = ve::voxel::tilepos { 1, 1, 1 };
            #else
                auto load_radius = ve::voxel::tilepos { 5, 5, 5 };
            #endif

            voxel.get_space()->add_chunk_loader(ve::make_shared<ve::voxel::point_loader<>>(load_where, load_radius));
        }


        static void remote_initializer(ve::registry& owner, entt::entity entity, const ve::voxel_component& cmp) {
            owner.set_component(entity, pbr_render_tag { });
            owner.set_component(entity, ve::mesh_component { .buffer = cmp.get_space()->get_vertex_buffer() });
            owner.set_component(entity, ve::update_component { &world::update_fn });
        }


        static void update_fn(const ve::invocation_context& ctx, ve::nanoseconds dt) {
            ctx.registry->get_component<ve::voxel_component>(ctx.entity).get_space()->update(dt);
        }


        ve::update_component VE_COMPONENT(update) = ve::update_component { &world::update_fn };
        ve::voxel_component VE_COMPONENT(voxel) = ve::voxel_component { true, get_world_generator() };
        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
    };
}