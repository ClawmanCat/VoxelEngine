#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/voxel/voxel_settings.hpp>
#include <VoxelEngine/dependent/actor.hpp>
#include <VoxelEngine/utility/direction.hpp>


namespace ve {
    struct tile_parameters {
        std::string name;
        bool colliding = true;
        bool rendered  = true;
        tile_metadata num_states = 1;
        ve_default_actor(owner);
    };
    
    
    class tile {
    public:
        explicit tile(tile_parameters&& params);
        virtual ~tile(void);
        
        tile(const tile&) = delete;
        tile& operator=(const tile&) = delete;
        
        tile(tile&& o) { *this = std::move(o); }
        tile& operator=(tile&&);
        
        
        operator tile_data(void) const {
            return tile_data { .tile = id, .metadata = default_state };
        }
    
    
        // Creates the mesh for this tile, given a list of sides from which it is occluded.
        // For performance reasons, an empty mesh is provided to store the result of this function to,
        // which may already have some pre-allocated memory associated with it.
        // This method can be overridden for tiles that have a custom meshing function.
        // Note: the result of this method may be cached and should be constant for a given state.
        virtual void mesh(voxel_mesh_t& allocated_mesh, tile_metadata state, direction occluded_sides) const ;
        
        // Does the presence of this tile block vision of an adjacent tiles connecting face on the given side?
        // This method should be overridden for tiles that use a custom meshing method that does not produce a full block.
        virtual bool occludes_side(direction dir) const;
        
        // This method can be overridden for tiles that have different textures on different sides.
        // Note: the result of this method may be cached and should be constant for a given state.
        virtual const fs::path& get_texture_for_side(direction dir, tile_metadata state) const;
    
        // This method can be overridden for tiles that have the same texture on each side.
        // Note: the result of this method may be cached and should be constant for a given state.
        virtual const fs::path& get_texture(tile_metadata state) const;
        
        
        VE_GET_CREF(name);
        VE_GET_BOOL_IS(colliding);
        VE_GET_BOOL_IS(rendered);
        VE_GET_VAL(num_states);
        VE_GET_VAL(owner);
        
    private:
        std::string name;
        bool colliding;
        bool rendered;
        tile_metadata num_states;
        actor_id owner;
        
        friend class tile_registry;
        mutable tile_id id = invalid_tile;
        mutable tile_metadata default_state = 0;
        
    protected:
        fs::path texture_path;
    };
}