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
            hash_map<tilepos, u16> new_loaded;

            spatial_foreach(
                [&] (const tilepos& pos) {
                    auto distance_to_entity = DistanceMetric(pos, where);

                    if (distance_to_entity <= DistanceMetric(tilepos { 0 }, range)) {
                        new_loaded.emplace(
                            pos,
                            std::min(priority::NORMAL, u16(priority::NORMAL - distance_to_entity))
                        );
                    }
                },
                where,
                range
            );

            update_loaded(space, std::move(new_loaded));
        }


        void stop_loading(voxel_space* space) override {
            if (!loaded.empty()) update_loaded(space, { });
        }


        VE_GET_VAL(range);
        VE_GET_VAL(entity);
        VE_GET_VAL(registry);
    private:
        hash_map<tilepos, u16> loaded;
        tilepos range;

        entt::entity entity;
        const registry* registry;


        void update_loaded(voxel_space* space, hash_map<tilepos, u16> new_loaded) {
            for (const auto& [pos, priority] : loaded) {
                if (!new_loaded.contains(pos)) unload(space, pos);
            }

            for (const auto& [pos, priority] : new_loaded) {
                if (!loaded.contains(pos)) load(space, pos, priority);
            }

            loaded = std::move(new_loaded);
        }
    };
}