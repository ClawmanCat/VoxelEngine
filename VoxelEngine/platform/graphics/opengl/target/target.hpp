#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/context.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>


namespace ve::graphics {
    class target {
    public:
        target(shared<pipeline> source) : source(std::move(source)) {}
        virtual ~target(void) = default;
        
        ve_swap_move_only(target, source);
        
        
        virtual void draw(void) {
            source->draw();
        }
        
        void set_target(shared<pipeline> src) {
            source = std::move(src);
        }
    private:
        shared<pipeline> source;
    };
}