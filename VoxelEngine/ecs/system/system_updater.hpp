#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/function_component.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    template <
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<>,
        template <typename System> typename... Mixins
    > class system_updater : public system<
        system_updater<RequiredTags, ExcludedTags, Mixins...>,
        meta::pack_ops::merge_all<RequiredTags, update_component>,
        ExcludedTags,
        deduce_component_access,
        Mixins...
    > {
    public:
        explicit system_updater(u16 priority = priority::HIGH) : priority(priority) {}


        template <typename Component> constexpr static u8 access_mode_for_component(void) {
            return (u8) system_access_mode::READ_CMP;
        }


        u16 get_priority(void) const {
            return priority;
        }


        void on_system_update(registry& owner, view_type view, nanoseconds dt) {
            VE_PROFILE_FN();


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