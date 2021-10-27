#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/space/voxel_space.hpp>

#include <glm/gtx/norm.hpp>


namespace ve::voxel {
    namespace distance_metrics {
        // L1 Distance (Taxicab Distance)
        constexpr inline auto L1 = [](const auto& a, const auto& b) { return glm::l1Norm(a, b); };
        // L2 Distance (Euclidean Distance)
        constexpr inline auto L2 = [](const auto& a, const auto& b) { return glm::l2Norm(a, b); };
    }


    class chunk_loader {
    public:
        virtual ~chunk_loader(void) = default;

        virtual void start_loading(voxel_space* space) {}
        virtual void stop_loading(voxel_space* space) {}
        virtual void update(voxel_space* space) {}

    protected:
        static void load(voxel_space* space, const tilepos& chunk) {
            space->load_chunk(chunk);
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


    template <auto DistanceMetric = distance_metrics::L2>
    class point_loader : public chunk_loader {
    public:
        point_loader(const tilepos& where, const tilepos& range) : where(where), range(range) {}


        void start_loading(voxel_space* space) override {
            spatial_foreach(
                [&] (const tilepos& pos) {
                    if (DistanceMetric(pos, where) <= DistanceMetric(tilepos { 0 }, range)) load(space, pos);
                },
                where,
                range
            );
        }


        void stop_loading(voxel_space* space) override {
            spatial_foreach(
                [&] (const tilepos& pos) {
                    if (DistanceMetric(pos, where) <= DistanceMetric(tilepos { 0 }, range)) unload(space, pos);
                },
                where,
                range
            );
        }

    private:
        tilepos where, range;
    };


    class single_chunk_loader : public chunk_loader {
    public:
        explicit single_chunk_loader(const tilepos& where) : where(where) {}

        void start_loading(voxel_space* space) override {
            load(space, where);
        }

        void stop_loading(voxel_space* space) override {
            unload(space, where);
        }
    private:
        tilepos where;
    };
}