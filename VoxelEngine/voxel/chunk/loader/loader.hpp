#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/space/voxel_space.hpp>

#include <glm/gtx/norm.hpp>


namespace ve::voxel {
    namespace distance_metrics {
        // L1 Distance (Taxicab Distance)
        constexpr inline auto L1 = [](const auto& a, const auto& b) { return glm::l1Norm((vec3f) a, (vec3f) b); };
        // L2 Distance (Euclidean Distance)
        constexpr inline auto L2 = [](const auto& a, const auto& b) { return glm::l2Norm((vec3f) a, (vec3f) b); };
    }


    class chunk_loader {
    public:
        virtual ~chunk_loader(void) = default;

        virtual void start_loading(voxel_space* space) {}
        virtual void stop_loading(voxel_space* space) {}
        virtual void update(voxel_space* space) {}

    protected:
        static void load(voxel_space* space, const tilepos& chunk, u16 priority = priority::LOWEST) {
            space->load_chunk(chunk, priority);
        }

        static void unload(voxel_space* space, const tilepos& chunk) {
            space->unload_chunk(chunk);
        }


        template <typename Pred> static void spatial_foreach(Pred pred, const tilepos& where, const tilepos& range) {
            for (auto x = where.x - range.x; x <= where.x + range.x; ++x) {
                for (auto y = where.y - range.y; y <= where.y + range.y; ++y) {
                    for (auto z = where.z - range.z; z <= where.z + range.z; ++z) {
                        std::invoke(pred, tilepos { x, y, z });
                    }
                }
            }
        }
    };


    // Loads chunks around the given point.
    template <auto DistanceMetric = distance_metrics::L2>
    class point_loader : public chunk_loader {
    public:
        point_loader(const tilepos& center, const tilepos& range) : center(center), range(range) {}


        void start_loading(voxel_space* space) override {
            spatial_foreach(
                [&] (const tilepos& pos) {
                    if (DistanceMetric(pos, center) <= DistanceMetric(tilepos { 0 }, range)) load(space, pos);
                },
                center,
                range
            );
        }


        void stop_loading(voxel_space* space) override {
            spatial_foreach(
                [&] (const tilepos& pos) {
                    if (DistanceMetric(pos, center) <= DistanceMetric(tilepos { 0 }, range)) unload(space, pos);
                },
                center,
                range
            );
        }


        VE_GET_VAL(center);
        VE_GET_VAL(range);
    private:
        tilepos center, range;
    };


    // Loads the provided list of chunks.
    class multi_chunk_loader : public chunk_loader {
    public:
        explicit multi_chunk_loader(hash_set<tilepos> loaded_chunks = {}) : loaded_chunks(std::move(loaded_chunks)) {}


        void start_loading(voxel_space* space) override {
            for (const auto& pos : loaded_chunks) load(space, pos);
        }

        void stop_loading(voxel_space* space) override {
            for (const auto& pos : loaded_chunks) unload(space, pos);
        }

        void update(voxel_space* space) override {
            for (const auto& pending : pending_loads) {
                auto [it, success] = loaded_chunks.emplace(pending);
                if (success) load(space, pending);
            }

            pending_loads.clear();
        }


        void add_chunk(const tilepos& where) {
            pending_loads.push_back(where);
        }

        void remove_chunk(const tilepos& where) {
            loaded_chunks.erase(where);
            std::erase(pending_loads, where);
        }


        // Note: only returns chunks that are actually loaded, not pending ones.
        VE_GET_CREF(loaded_chunks);
    private:
        hash_set<tilepos> loaded_chunks;
        std::vector<tilepos> pending_loads;
    };
}