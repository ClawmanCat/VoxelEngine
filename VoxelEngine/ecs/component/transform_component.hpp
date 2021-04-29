#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/utility/serialize/serialize.hpp>


namespace ve {
    struct transform_component :
        public component<transform_component, side::BOTH>
        //public auto_binary_serializable<transform_component>
    {
        explicit transform_component(
            const vec3f& position = vec3f { 0 },
            const vec3f& linear_velocity = vec3f { 0 },
            const quatf& rotation = quatf { },
            const float angular_velocity = 0.0f
        ) :
            position(position),
            linear_velocity(linear_velocity),
            rotation(rotation),
            angular_velocity(angular_velocity)
        {}
        
        
        vec3f position;
        vec3f linear_velocity;
        
        quatf rotation;
        float angular_velocity;
    };
}