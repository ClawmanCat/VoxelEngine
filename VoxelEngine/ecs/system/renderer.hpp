#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/renderable_component.hpp>
#include <VoxelEngine/platform/platform_include.hpp>
#include VE_GRAPHICS_INCLUDE(pipeline/pipeline.hpp)


namespace ve {
    class renderer : public system<renderer, side::CLIENT, meta::pack<renderable_component>> {
    public:
        explicit renderer(shared<graphics::pipeline>&& pipeline) : pipeline(std::move(pipeline)) {}
        
        
        void update(view_type& view, microseconds dt) {
            pipeline->clear();
            
            for (auto entity : view) {
                auto& component = view.get<renderable_component>(entity);
                
                for (auto& [shader, buffer] : component.buffers) {
                    pipeline->add_buffer(copy(shader), copy(buffer));
                }
            }
        }
        
        
        [[nodiscard]] graphics::pipeline& get_pipeline(void) {
            return *pipeline;
        }
    private:
        shared<graphics::pipeline> pipeline;
    };
}