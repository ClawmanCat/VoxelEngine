#pragma once

#include <VEDemoGame/core/core.hpp>
#include <VEDemoGame/tile/tiles.hpp>


namespace demo_game {
    class world : public ve::entity<world, ve::side::CLIENT> {
    public:
        using base = ve::entity<world, ve::side::CLIENT>;
        using base::base;
        
        
        // Note: components arent valid until the entity is added to a scene,
        // so constructors cannot be used for this.
        void init(void) {
            // Add the voxel space's mesh to be part of this entity's mesh.
            mesh.buffers.push_back(gfx::shader_buffer_pair {
                gfx::shader_library::get_shader("atlas_texture_3d"s),
                voxels.get_mesh()
            });
            
            // Load some chunks in the voxel space.
            voxels.toggle_mesh_updates(false);
            voxels.add_loader(std::make_shared<ve::point_loader<ve::distance_functions::L1<ve::chunkpos>{ }>>(
                &voxels, ve::vec3i { 0 }, 3
            ));
            voxels.toggle_mesh_updates(true);
        }
        
        
        VE_PROPER_COMPONENT(
            voxels,
            ve::voxel_space,
            std::make_unique<ve::flatland_generator>(
                ve::terrain_layers({
                    { -1, tile_stone },
                    {  0, tile_grass }
                }),
                *ve::tiles::TILE_VOID
            )
        );
        
        
        VE_PROPER_COMPONENT(mesh, ve::renderable_component);
        VE_PROPER_COMPONENT(transform, ve::transform_component);
    };
}