#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/scheduling2/scheduling_strategy.hpp>
#include <VoxelEngine/ecs/system/scheduling2/scheduling_strategy_selector.hpp>


namespace ve::ecs::schedule {
    /** Strategy selector for a strategy that does not change over time. */
    template <typename R> class constant_strategy : public scheduling_strategy_selector<R> {
    public:
        explicit constant_strategy(unique<constant_strategy<R>> strategy) :
            scheduling_strategy_selector<R>(false),
            strategy(std::move(strategy))
        {}


        unique<scheduling_strategy<R>> update_strategy(const scheduler_info<R>& scheduler, strategy_change_reason reason, const task_info<R>* task) const override {
            return std::exchange(strategy, nullptr);
        }
    private:
        unique<scheduling_strategy<R>> strategy;
    };
}