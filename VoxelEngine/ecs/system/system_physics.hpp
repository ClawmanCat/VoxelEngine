#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>
#include <VoxelEngine/ecs/component/motion_component.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    struct dont_apply_physics_by_default_tag {};


    // TODO: Right now this system just moves entities according to their velocity.
    // Actual physics still needs to be added.
    template <
        // Tags can be used to split entities across different updaters.
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<dont_apply_physics_by_default_tag>
    > class system_physics : public system<
        system_physics<RequiredTags, ExcludedTags>,
        typename RequiredTags
            ::template append<transform_component, motion_component>,
        ExcludedTags
    > {
    public:
        explicit system_physics(u16 priority = priority::HIGH) : priority(priority) {}


        u16 get_priority(void) const {
            return priority;
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            const float dt_seconds = float(dt.count()) / 1e9f;

            for (auto entity : view) {
                auto& transform = view.template get<transform_component>(entity);
                auto& motion    = view.template get<motion_component>(entity);


                transform.position += motion.linear_velocity * dt_seconds;
                transform.rotation = glm::normalize(glm::mix(glm::identity<quatf>(), motion.angular_velocity, dt_seconds)) * transform.rotation;
            }
        }

    private:
        u16 priority;
    };
}