#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/tile/tile_data.hpp>
#include <VoxelEngine/graphics/texture/texture_type.hpp>
#include <VoxelEngine/utility/direction.hpp>

#include <magic_enum.hpp>


namespace ve::voxel {
    class tile_registry;


    class tile {
    public:
        struct arguments {
            std::string name;
            std::string texture_name = name;

            tile_metadata_t num_states = 1;
            bool rendered = true;
            bool transparent = false;
            bool solid = rendered;
        };


        explicit tile(const arguments& args);
        virtual ~tile(void) = default;

        ve_immovable(tile);


        // Given a set of visible sides of this tile, appends vertices for those sides to the given mesh.
        // visible_sides is a bitfield where bit n corresponds to ve::directions[n].
        virtual void append_mesh(tile_mesh& dest, u8 visible_sides, tile_metadata_t meta) const;


        virtual const fs::path& get_texture_for_side(direction_t direction, gfx::texture_type type, tile_metadata_t meta) const {
            return textures[(u32) type];
        }


        bool is_stateless(void) const {
            return num_states == 1;
        }


        // Returns whether or not a neighbouring tile on the given side has its face occluded by this tile.
        virtual bool occludes_side(direction_t dir, tile_metadata_t meta) const {
            return rendered && !transparent;
        }


        VE_GET_CREF(name);
        VE_GET_CREF(textures);
        VE_GET_BOOL_IS(rendered);
        VE_GET_BOOL_IS(transparent);
        VE_GET_BOOL_IS(solid);
        VE_GET_VAL(num_states);
    private:
        std::string name;
        std::array<fs::path, magic_enum::enum_count<gfx::texture_type>()> textures;

        tile_metadata_t num_states;
        bool rendered;
        bool transparent;
        bool solid;


        // Store some of the tile registry data intrusively in the tile for cheap lookup.
        friend class tile_registry;
        mutable tile_id_t id = invalid_tile_id;
        mutable tile_id_t metastate = invalid_tile_id;
        mutable bool removable = true;
    };
}