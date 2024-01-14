#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/ecs/system/system_traits.hpp>

#include <chrono>


namespace ve::ecs {
    using system_id = u32;


    /** CRTP base class for all ECS systems. */
    template <typename Derived, system_traits Traits> class system {
    public:
        using self_type     = system<Derived, Traits>;
        using system_traits = Traits;


        /**
         * Callback invoked when (just after) the system is added to a registry.
         * @param registry The registry the system was added to.
         */
        template <typename R> void on_added(R& registry) {
            VE_TRY_TEMPLATE_CRTP_CALL(self_type, Derived, on_added<R>, (registry));
        }


        /**
         * Callback invoked when (just before) the system is removed from the registry.
         * @param registry The registry the system was removed from.
         */
        template <typename R> void on_removed(R& registry) {
            VE_TRY_TEMPLATE_CRTP_CALL(self_type, Derived, on_removed<R>, (registry));
        }


        /**
         * Invokes the system.
         * @param view A view of all entities matched by the system and the relevant components, as defined in the system's traits.
         * @param dt The amount of time that has passed since the last engine tick.
         * @param prev A timestamp indicating the time of the last tick the system was invoked.
         * @param now A timestamp indicating the time of the current tick.
         */
        template <typename View> void operator()(View&& view, time::duration dt, const time::tick_timestamp& prev, const time::tick_timestamp& now) {
            VE_CRTP_TEMPLATE_CALL(self_type, Derived, operator()<View>, view, dt);
        }


        /**
         * Should return true if the system should be invoked for the current tick.
         * @param prev A timestamp indicating the time of the last tick the system was invoked.
         * @param now A timestamp indicating the time of the current tick.
         * @return True if the system should be invoked this tick, or false otherwise.
         */
        [[nodiscard]] bool should_invoke(const time::tick_timestamp& prev, const time::tick_timestamp& now) const {
            return VE_TRY_CRTP_CALL(self_type, Derived, should_invoke, (prev, now), true);
        }
    };


    template <typename System> concept ecs_system = meta::is_template_v<system, System>;
}