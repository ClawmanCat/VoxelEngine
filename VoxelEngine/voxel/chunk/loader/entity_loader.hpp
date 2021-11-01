#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/voxel/chunk/loader/loader.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>


namespace ve::voxel {
    template <auto DistanceMetric = distance_metrics::L2>
    class entity_loader : public chunk_loader {
    public:
        entity_loader(entt::entity entity, const registry& registry, const tilepos& range) :
            range(range),
            entity(entity),
            registry(&registry)
        {}


        void update(voxel_space* space) override {
            if (!registry->template has_component<transform_component>(entity)) {
                stop_loading(space);
                return;
            }

            auto where = (tilepos) (registry->template get_component<transform_component>(entity).position / ((f32) voxel_settings::chunk_size));
            hash_set<tilepos> new_loaded;

            spatial_foreach(
                [&] (const tilepos& pos) {
                    if (DistanceMetric(pos, where) <= DistanceMetric(tilepos { 0 }, range)) new_loaded.insert(pos);
                },
                where,
                range
            );

            update_loaded(space, std::move(new_loaded));
        }


        void stop_loading(voxel_space* space) override {
            if (!loaded.empty()) update_loaded(space, { });
        }
    private:
        hash_set<tilepos> loaded;
        tilepos range;

        entt::entity entity;
        const registry* registry;


        void update_loaded(voxel_space* space, hash_set<tilepos> new_loaded) {
            for (const auto& pos : loaded) {
                if (!new_loaded.contains(pos)) unload(space, pos);
            }

            for (const auto& pos : new_loaded) {
                if (!loaded.contains(pos)) load(space, pos);
            }

            loaded = std::move(new_loaded);
        }
    };
}