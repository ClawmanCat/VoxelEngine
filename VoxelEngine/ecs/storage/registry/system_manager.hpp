#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/utility/services/logger.hpp>
#include <VoxelEngine/ecs/system/scheduling/system_scheduler.hpp>


namespace ve::ecs {
    class system_manager {
    public:
        /**
         * Constructs a system manager with the given settings.
         * @param starting_tick The tick number to start at.
         *  This value cannot be zero, as the epoch timestamp must always compare less than the current timestamp.
         * @param minimum_dt The minimum amount of time simulated by the system manager.
         *  If the current simulated duration is less than the given duration, it will be clamped to the minimum duration,
         *  and the system manager will block for the remaining amount of time (yielding the thread).
         * @param maximum_dt The maximum amount of time simulated by the system manager.
         *  If the current simulated duration is more than the given duration, it will be clamped to the maximum duration.
         *  This will appear in-game as if time is slowing down.
         * @param default_dt The amount of time simulated by the system manager during the first tick.
         *  For all subsequent ticks, the duration of the previous tick is used to decide how much time to simulate.
         */
        explicit system_manager(
            u64 starting_tick         = 1,
            time::duration minimum_dt = std::chrono::milliseconds { 1   },
            time::duration maximum_dt = std::chrono::milliseconds { 250 },
            time::duration default_dt = std::chrono::milliseconds { 10  }
        ) :
            tick(starting_tick),
            current_dt(default_dt),
            min_dt(minimum_dt),
            max_dt(maximum_dt)
        {
            VE_ASSERT(starting_tick > 0,        "Cannot start system manager at tick 0.");
            VE_ASSERT(maximum_dt <= maximum_dt, "Minimum timestep cannot be less than maximum timestep.");
            VE_ASSERT(default_dt >= minimum_dt, "Default timestep cannot be less than the minimum timestep.");
            VE_ASSERT(default_dt <= maximum_dt, "Default timestep cannot be more than the maximum timestep.");
        }


        /** Invokes all systems in the ECS that should be invoked this tick. */
        void update(void) {
            time::tick_timestamp now {
                .when = time::clock::now(),
                .tick = tick
            };

            auto dt = std::clamp(current_dt, min_dt, max_dt);
            scheduler.invoke(dt, now);

            current_dt = time::clock::now() - now.when;
            if      (current_dt < min_dt) std::this_thread::sleep_until(now.when + min_dt);
            else if (current_dt > max_dt) get_service<engine_logger>().warning("ECS cannot keep up! (Current tick took {}, but simulated maximum dt of {})", current_dt, max_dt);

            ++tick;
        }


        /**
         * Adds a system to the scheduler and assigns it an unique ID.
         * @tparam Derived Derived registry class. This function is wrapped by the registry to bind this parameter.
         * @param system The system to add to the scheduler.
         * @return A pair of the ID of the newly added system and a reference to it.
         */
        template <typename Derived, typename System> std::pair<System&, system_id> add_system(System system) {
            system_id id = scheduler.add_system(std::move(system), static_cast<Derived*>(this));

            return {
                *(scheduler.template get_system<System>(id)),
                id
            };
        }


        /**
         * Removes a system from the scheduler.
         * @param system The ID of the system to remove.
         * @return True if the system was removed from the scheduler, or false if no system with the given ID existed.
         */
        bool remove_system(system_id system) {
            return scheduler.remove_system(system);
        }


        /**
         * Takes a system from the scheduler. This is equivalent to moving the system and then removing the now moved-out-of value.
         * @tparam System The type of the system to take out of the scheduler.
         * @param system The ID of the system to take out of the scheduler.
         * @return The system if it was present, or std::nullopt otherwise.
         * @post The system is no longer present in the scheduler after calling this method.
         */
        template <typename System> std::optional<System> take_system(system_id system) {
            return scheduler.template take_system<System>(system);
        }


        /**
         * Returns a pointer to the system with the given ID, if a system with the given ID and correct type exists.
         * @tparam System The type of the system.
         * @param system The ID of the system.
         * @return A pointer to the system if it exists, or nullptr.
         */
        template <typename System> [[nodiscard]] System* get_system(system_id system) {
            return scheduler.template get_system<System>(system);
        }


        /** @copydoc get_system */
        template <typename System> [[nodiscard]] const System* get_system(system_id system) const {
            return scheduler.template get_system<System>(system);
        }


        /** Returns true if a system with the given ID exists. */
        [[nodiscard]] bool has_system(system_id system) const {
            return scheduler.has_system(system);
        }


        VE_GET_VALS(tick, current_dt);
        VE_GET_SET_VALS(min_dt, max_dt);
        VE_GET_CREFS(scheduler);
    private:
        schedule::system_scheduler scheduler;

        u64 tick;
        time::duration current_dt, min_dt, max_dt;
    };
}