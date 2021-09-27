#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/mesh_component.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(pipeline/pipeline.hpp)


namespace ve {
    struct dont_render_by_default_tag {};


    template <
        // Tags can be used to split meshes across different renderers.
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<dont_render_by_default_tag>,
        // Typically, rendering is the last operation to be done, so the displayed entities are not lagging behind by one frame.
        u16 Priority = priority::LOWEST
    > class system_renderer : public system<
        system_renderer<RequiredTags, ExcludedTags>,
        typename RequiredTags
            ::template append<mesh_component, transform_component>
            ::unique,
        ExcludedTags,
        Priority
    > {
    public:
        using base = system<
            system_renderer<RequiredTags, ExcludedTags>,
            typename RequiredTags
                ::template append<mesh_component, transform_component>
                ::unique,
            ExcludedTags,
            Priority
        >;


        system_renderer(void) = default;
        explicit system_renderer(const shared<gfxapi::pipeline>& pipeline) : pipeline(pipeline) {}


        void update(typename base::view_type view, nanoseconds dt) {
            pipeline->bind();

            for (auto entity : view) {
                // TODO: Set transform uniform.
                pipeline->draw(*(view.template get<mesh_component>(entity).mesh));
            }

            pipeline->unbind();
        }

    private:
        shared<gfxapi::pipeline> pipeline;

    public:
        VE_GET_SET_VAL(pipeline);
    };
}