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
                gfx::shader_library::instance().get_shader("atlas_texture_3d"s),
                voxels.get_mesh()
            });
            
            // Load some chunks in the voxel space.
            voxels.add_loader(std::make_shared<ve::point_loader<ve::distance_functions::L1<ve::chunkpos>{ }>>(
                &voxels, ve::vec3i { 0 }, 5
            ));
        }
        
        
        void VE_FUNCTION_COMPONENT(update, SERVER, ve::microseconds dt) {
            // TODO: Use events for this.
            voxels.update_mesh();
        }
        
        
        ve::voxel_space VE_COMPONENT(voxels) = ve::voxel_space {
            std::make_unique<ve::flatland_generator>(
                ve::terrain_layers({
                    { -1, tile_stone },
                    {  0, tile_grass }
                }),
                *ve::tiles::TILE_VOID
            )
        };
    
    
        ve::renderable_component VE_COMPONENT(mesh);
        ve::transform_component VE_COMPONENT(transform);
    };
}