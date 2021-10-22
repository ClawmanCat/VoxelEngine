#pragma once

#include <VEDemoGame/game.hpp>
#include <VEDemoGame/tile/tiles.hpp>
#include <VEDemoGame/component/render_tag.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/voxel/voxel.hpp>


namespace demo_game {
    inline const auto& get_world_layout(void) {
        using fg = ve::voxel::flatland_generator;

        static fg::world_layers result;
        result.set_sky(ve::voxel::tiles::TILE_AIR);

        result.add_layer(-1, tiles::TILE_STONE);
        result.add_layer(0,  tiles::TILE_GRASS);

        return result;
    };


    class world : public ve::static_entity {
    public:
        explicit world(ve::registry& registry) :
            ve::static_entity(registry),
            space(ve::make_unique<ve::voxel::flatland_generator>(get_world_layout()))
        {
            mesh.buffer = space.get_vertex_buffer();

            auto load_where  = ve::voxel::tilepos { 0 };
            auto load_radius = ve::voxel::tilepos { 2 };
            space.add_chunk_loader(ve::make_shared<ve::voxel::point_loader<ve::voxel::distance_metrics::L1>>(load_where, load_radius));
        }


        void VE_COMPONENT_FN(update)(ve::nanoseconds dt) {
            space.update(dt);
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::mesh_component VE_COMPONENT(mesh) = ve::mesh_component { };
        render_tag_pbr VE_COMPONENT(render_tag) = render_tag_pbr { };

        ve::voxel::voxel_space space;
    };
}