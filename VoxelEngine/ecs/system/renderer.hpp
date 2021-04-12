#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/renderable_component.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/platform/platform_include.hpp>
#include VE_GRAPHICS_INCLUDE(pipeline/pipeline.hpp)


namespace ve {
    class renderer : public system<renderer, side::CLIENT, meta::pack<renderable_component, transform_component>> {
    public:
        explicit renderer(shared<graphics::pipeline>&& pipeline) : pipeline(std::move(pipeline)) {}
        
        
        void update(view_type& view, microseconds dt) {
            pipeline->clear();
            
            for (auto entity : view) {
                auto [renderable, transform] = view.get<renderable_component, transform_component>(entity);
                
                for (auto& [shader, buffer] : renderable.buffers) {
                    buffer->set_uniform_value("transform"s, translation_matrix(transform.position));
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