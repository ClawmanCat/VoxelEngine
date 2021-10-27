#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/tile_provider.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/traits/evaluate_if_valid.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(vertex/vertex_buffer.hpp)


namespace ve::voxel {
    class chunk_loader;
    class chunk_generator;


    namespace detail {
        using vertex_t = tile_mesh::vertex_t;
        using index_t  = ve_type_if_valid(tile_mesh::index_t);
        using buffer_t = gfxapi::compound_vertex_buffer<vertex_t, index_t>;

        using subbuffer_t = std::conditional_t<
            tile_mesh::indexed,
            // Cannot directly reference indexed_vertex_buffer with non-scalar index type, even if using ve_type_if_valid.
            gfxapi::indexed_vertex_buffer<vertex_t, std::conditional_t<tile_mesh::indexed, index_t, u32>>,
            gfxapi::unindexed_vertex_buffer<vertex_t>
        >;
    }


    class voxel_space : public tile_provider<voxel_space> {
    public:
        explicit voxel_space(unique<chunk_generator>&& generator);
        virtual ~voxel_space(void) = default;
        ve_rt_move_only(voxel_space);

        virtual void update(nanoseconds dt);

        const tile_data& get_data(const tilepos& where) const;
        void set_data(const tilepos& where, const tile_data& td);
        bool is_loaded(const tilepos& chunkpos) const;

        void add_chunk_loader(shared<chunk_loader> loader);
        void remove_chunk_loader(const shared<chunk_loader>& loader);

        VE_GET_CREF(vertex_buffer);


        constexpr static tilepos to_chunkpos(const tilepos& worldpos) {
            return worldpos >> tilepos::value_type(lsb(voxel_settings::chunk_size));
        }

        constexpr static tilepos to_worldpos(const tilepos& chunkpos, const tilepos& offset = tilepos { 0 }) {
            return (chunkpos << tilepos::value_type(lsb(voxel_settings::chunk_size))) + offset;
        }

        constexpr static tilepos to_localpos(const tilepos& worldpos) {
            return worldpos - to_worldpos(to_chunkpos(worldpos));
        }
    private:
        struct per_chunk_data {
            unique<chunk> chunk;
            shared<detail::subbuffer_t> subbuffer;
            detail::buffer_t::buffer_handle handle;
            bool needs_meshing;

            std::size_t load_count;
        };


        hash_map<tilepos, per_chunk_data> chunks;
        shared<chunk_generator> generator;
        hash_set<shared<chunk_loader>> chunk_loaders;

        shared<detail::buffer_t> vertex_buffer;


        friend class chunk_loader;
        void load_chunk(const tilepos& where);
        void unload_chunk(const tilepos& where);
    };
}