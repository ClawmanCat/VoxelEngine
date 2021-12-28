#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/settings.hpp>
#include <VoxelEngine/voxel/chunk/chunk.hpp>
#include <VoxelEngine/voxel/tile_provider.hpp>
#include <VoxelEngine/utility/priority.hpp>
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


    // A chunk and its neighbours.
    struct chunk_neighbourhood {
        const class chunk* chunk;
        std::array<const class chunk*, directions.size()> neighbours;
    };


    class voxel_space : public tile_provider<voxel_space>, public std::enable_shared_from_this<voxel_space> {
    public:
        // RAII object that keeps the chunk and the space that contains it alive and locked while it exists.
        class chunk_locker {
        public:
            chunk_locker(shared<voxel_space> space, std::vector<tilepos> where);
            ~chunk_locker(void);

            ve_swap_move_only(chunk_locker, space, loader);

            VE_GET_VAL(space);
            VE_GET_VAL(loader);
        private:
            shared<voxel_space> space = nullptr;
            shared<chunk_loader> loader = nullptr;
        };


        ve_shared_only(voxel_space, unique<chunk_generator>&& generator) :
            vertex_buffer(detail::buffer_t::create())
        {
            // chunk_generator is incomplete here, so this must be done in the CPP file.
            init(std::move(generator));
        }

        virtual ~voxel_space(void) = default;
        ve_rt_move_only(voxel_space);


        virtual void update(nanoseconds dt);

        const tile_data& get_data(const tilepos& where) const;
        void set_data(const tilepos& where, const tile_data& td);
        bool is_loaded(const tilepos& chunkpos) const;

        void add_chunk_loader(shared<chunk_loader> loader);
        void remove_chunk_loader(const shared<chunk_loader>& loader);

        VE_GET_CREF(vertex_buffer);
    private:
        struct per_chunk_data {
            unique<chunk> chunk;

            shared<detail::subbuffer_t> subbuffer;
            detail::buffer_t::buffer_handle handle;
            enum { NEEDS_MESHING, MESHING, MESHED } mesh_status;
            u32 most_recent_mesh_task = 0;

            std::size_t load_count;
            u16 load_priority;
        };


        hash_map<tilepos, per_chunk_data> chunks;
        shared<chunk_generator> generator;
        hash_set<shared<chunk_loader>> chunk_loaders;

        shared<detail::buffer_t> vertex_buffer;
        unique<std::atomic_uint32_t> ongoing_mesh_tasks = make_unique<std::atomic_uint32_t>(0); // Use pointer to keep class movable.


        void init(unique<chunk_generator>&& generator);
        void update_meshes(void);

        // TODO: Use access facade?
        friend class chunk_loader;
        void load_chunk(const tilepos& where, u16 priority = priority::LOWEST);
        void unload_chunk(const tilepos& where);
    };
}