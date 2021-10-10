#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/function_component.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    struct dont_update_by_default_tag {};

    template <
        // Tags can be used to split entities across different updaters.
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<dont_update_by_default_tag>
    > class system_updater : public system<
        system_updater<RequiredTags, ExcludedTags>,
        typename RequiredTags
            ::template append<update_component>,
        ExcludedTags
    > {
    public:
        explicit system_updater(u16 priority = priority::HIGH) : priority(priority) {}


        u16 get_priority(void) const {
            return priority;
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            invocation_context ctx { &owner, entt::null };

            for (auto entity : view) {
                const auto& updater = view.template get<update_component>(entity);
                VE_DEBUG_ASSERT(updater.value, "Entity has nullptr as its update function.");

                ctx.entity = entity;
                updater(ctx, dt);
            }
        }

    private:
        u16 priority;
    };
}