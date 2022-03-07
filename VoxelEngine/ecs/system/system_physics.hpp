#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>
#include <VoxelEngine/ecs/component/motion_component.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    // TODO: Right now this system just moves entities according to their velocity. Actual physics still needs to be added.
    template <
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<>,
        template <typename System> typename... Mixins
    > class system_physics : public system<
        system_physics<RequiredTags, ExcludedTags, Mixins...>,
        meta::pack_ops::merge_all<RequiredTags, transform_component, motion_component>,
        ExcludedTags,
        deduce_component_access,
        Mixins...
    > {
    public:
        // "max_dt" can be used to skip physics updates if the dt becomes very large.
        // This can be used to prevent applied movement from becoming too large if the game hangs for a significant amount of time.
        explicit system_physics(nanoseconds max_dt = 50ms, u16 priority = priority::HIGH) :
            max_dt(max_dt),
            priority(priority)
        {}


        u16 get_priority(void) const {
            return priority;
        }


        template <typename Component> constexpr static u8 access_mode_for_component(void) {
            if constexpr (std::is_same_v<Component, transform_component>) return (u8) system_access_mode::RW_CMP;
            if constexpr (std::is_same_v<Component, motion_component>)    return (u8) system_access_mode::READ_CMP;
        }

        constexpr static bool has_unsafe_side_effects(void) {
            return false;
        }


        void on_system_update(registry& owner, view_type view, nanoseconds dt) {
            if (dt > max_dt) dt = max_dt;


            const float dt_seconds = float(dt.count()) / 1e9f;

            for (auto entity : view) {
                auto& transform = view.template get<transform_component>(entity);
                auto& motion    = view.template get<motion_component>(entity);


                transform.position += motion.linear_velocity * dt_seconds;
                transform.rotation = glm::normalize(glm::mix(glm::identity<quatf>(), motion.angular_velocity, dt_seconds)) * transform.rotation;
            }
        }

    private:
        nanoseconds max_dt;
        u16 priority;

    public:
        VE_GET_SET_VAL(max_dt);
    };
}