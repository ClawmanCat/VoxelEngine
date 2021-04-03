#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/utility/serialize/serialize.hpp>


namespace ve {
    template <side Side = side::BOTH>
    struct transform_component :
        public component<transform_component<Side>, Side>,
        public auto_binary_serializable<transform_component<Side>>
    {
        vec3f position;
        vec3f linear_velocity;
        
        quatf rotation;
        float angular_velocity;
    };
}