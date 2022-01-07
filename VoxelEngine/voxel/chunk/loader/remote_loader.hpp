#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/chunk/generator/generator.hpp>
#include <VoxelEngine/voxel/chunk/loader/loader.hpp>


namespace ve::voxel {
    // Accepts chunk data from an external source and works as both a generator and a loader for said data.
    // If a space is managed through an external source in this way, it should not have any other chunk loaders than this one.
    class remote_loader : public chunk_generator, public chunk_loader {
    public:
        void stop_loading(voxel_space* space) override {
            for (const auto& pos : loaded) {
                unload(space, pos);
            }

            loaded.clear();
        }


        void update(voxel_space* space) override {
            for (const auto& pos : pending) {
                if (auto [it, success] = loaded.emplace(pos); success) load(space, pos);
            }

            pending.clear();


            for (const auto& pos : unloaded) {
                if (loaded.erase(pos)) unload(space, pos);
            }

            unloaded.clear();
        }


        unique<chunk> generate(const voxel_space* space, const tilepos& chunkpos) override {
            auto it = data.find(chunkpos);

            VE_ASSERT(
                it != data.end(),
                "remote_loader was used to generate chunk not received from remote. ",
                "The remote_loader class should not be used in conjunction with other chunk loaders."
            );


            auto chunk = make_unique<class chunk>();
            chunk->set_chunk_data(it->second);

            data.erase(it);

            return chunk;
        }


        void load_from_remote(const tilepos& where, const chunk::data_t& data) {
            this->data.emplace(where, data);
            pending.emplace(where);
        }


        void unload_from_remote(const tilepos& where) {
            data.erase(where);
            pending.erase(where);
            unloaded.emplace(where);
        }
    private:
        hash_set<tilepos> pending, loaded, unloaded;
        hash_map<tilepos, chunk::data_t> data;
    };
}