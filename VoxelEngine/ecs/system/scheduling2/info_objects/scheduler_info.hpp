#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/container/set_adapter.hpp>
#include <VoxelEngine/utility/threading/threading_utils.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/scheduling2/access_mode.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/task_info.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/typedefs.hpp>


namespace ve::ecs::schedule {
    /** Information about the state of the scheduler. */
    template <typename Registry> struct scheduler_info {
        /** The scheduler this object is associated with. */
        system_scheduler<Registry>* scheduler;
        /** The tasks currently stored within the scheduler. */
        hash_map<system_id, task_info<Registry>> tasks;
        /** For every sequence tag, a list of systems that have such a tag. */
        hash_map<sequence_tag, set_adapter<task_info<Registry>*>> sequence_tags;


        /** Adds the given task to the given scheduler info and updates the dependency graph. */
        void add_task(Registry* registry, task_info<Registry>&& task);
        /** Removes the given task from the given scheduler info and updates the dependency graph. */
        void remove_task(Registry* registry, const task_info<Registry>& task);
    };
}


#include <VoxelEngine/ecs/system/scheduling2/info_objects/scheduler_info.inl>