#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>


namespace ve {
    class movement : public system<movement, side::SERVER, meta::pack<transform_component>> {
    public:
        movement(void) = default;
        
        
        void update(view_type& view, microseconds dt) {
            for (auto e : view) {
                auto& component = view.get<transform_component>(e);
                component.position += (component.linear_velocity * (float(dt.count()) / 1e6f));
            }
        }
    };
}