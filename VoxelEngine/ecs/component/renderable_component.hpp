#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/graphics/shader_buffer_pair.hpp>


namespace ve {
    struct renderable_component : public component<renderable_component, side::CLIENT, component_serialization_mode::NONE> {
        std::vector<graphics::shader_buffer_pair> buffers;
    };
}