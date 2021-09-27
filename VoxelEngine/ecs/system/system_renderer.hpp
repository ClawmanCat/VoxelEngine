#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/mesh_component.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>


namespace ve {
    struct dont_render_by_default_tag {};


    template <
        // Tags can be used to split meshes across different renderers.
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<dont_render_by_default_tag>
    > class system_renderer : public system<
        system_renderer<RequiredTags, ExcludedTags>,
        typename RequiredTags
            ::template append<mesh_component, transform_component>
            ::unique,
        ExcludedTags
    > {
    public:
        explicit system_renderer(u16 priority = priority::LOW) : priority(priority) {}


        u16 get_priority(void) const {
            return priority;
        }


        void update(view_type view, nanoseconds dt) {
            VE_NOT_YET_IMPLEMENTED;
        }

    private:
        u16 priority;
    };
}