#pragma once

#include <VoxelEngine/platform/graphics/opengl/target/target.hpp>


namespace ve::graphics {
    class layerstack_target : public target {
    public:
        layerstack_target(shared<pipeline> source) : target(std::move(source)) {}
    };
}