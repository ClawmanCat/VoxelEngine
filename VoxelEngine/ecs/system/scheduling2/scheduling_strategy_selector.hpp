#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/ecs/system/scheduling2/scheduling_strategy.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/task_info.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/scheduler_info.hpp>

#include <variant>


namespace ve::ecs::schedule {
    enum class strategy_change_reason {
        /** Called just <b>after</b> a system is added. */
        SYSTEM_ADDED,
        /** Called just <b>before</b> a system is removed. */
        SYSTEM_REMOVED,
        /** Called just <b>after</b> the scheduler has ticked. */
        SCHEDULER_TICKED,
        /** Called just <b>after</b> the scheduler is constructed, <b>only if</b> no initial strategy is provided. */
        SCHEDULER_CONSTRUCTED
    };


    /**
     * Manages changes to the scheduling strategy of the system_scheduler.
     * @tparam R The type of the associated registry.
     */
    template <typename R> struct scheduling_strategy_selector {
        virtual ~scheduling_strategy_selector(void) = default;


        /**
         * Returns a new strategy of the strategy should change, or nullptr otherwise.
         * @param scheduler The scheduler whose strategy is being managed.
         * @param reason The reason the strategy might need to be changed.
         * @param task If the provided reason is SYSTEM_ADDED or SYSTEM_REMOVED, a pointer to the associated task_info, or nullptr otherwise.
         * @return Either a new strategy to replace the current strategy with, or nullptr if the current strategy should not change.
         */
        virtual unique<scheduling_strategy<R>> update_strategy(const scheduler_info<R>& scheduler, strategy_change_reason reason, const task_info<R>* task) const = 0;
    };
}