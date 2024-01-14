#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/scheduling/access_mode.hpp>
#include <VoxelEngine/ecs/system/scheduling/system_invoke_wrapper.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/utility/container/set_adapter.hpp>
#include <VoxelEngine/utility/container/map_adapter.hpp>
#include <VoxelEngine/utility/threading/threading_utils.hpp>

#include <vector>
#include <atomic>


namespace ve::ecs::schedule {
    class  system_scheduler;
    struct system_data;


    using component_id         = type_index_t;
    using seq_tag_id           = type_index_t;
    using component_access_map = map_adapter<component_id, access_mode, std::vector>;
    using system_set           = set_adapter<system_data*>;


    /** Data used within the @ref system_scheduler for each system. */
    struct system_data {
        unique<system_invoke_base> system;
        system_id id;

        // Priority = performance + priority of all dependents, recalculated each tick (stale = not yet recalculated).
        time::duration performance = time::duration { 1 };
        std::size_t priority = 1;
        bool priority_stale  = true;

        // Number of direct dependencies of this task that remain unfulfilled. Task can be added to available tasks once this is zero.
        atom<u32> remaining_dependencies = 0;

        component_access_map component_access;
        access_mode entity_access;

        // Tags of this system used to define its dependencies, dependents and blacklist.
        std::vector<seq_tag_id> tags;

        // Note: dependency/dependent list is symmetric: if A is a dependency of B, B is also a dependent of A.
        // Note: blacklist is symmetric: if A is in the blacklist of B, B is also in the blacklist of A.
        system_set dependencies, dependents, blacklist;
        // Number of systems in the blacklist that are currently being invoked.
        atom<u32> blacklist_count;

        // Equivalent to corresponding system_traits values.
        bool require_exclusive, require_main_thread;

        // Function to rebuild the list of dependencies/dependents/blacklists when a new system is added to the scheduler.
        fn<void, system_scheduler*, system_data&> rebuild_dependencies;
    };


    /** Comparator for sorting system_data objects based on their priority. */
    struct priority_comparator {
        bool operator()(const system_data* lhs, const system_data* rhs) const {
            // Compare using greater-than so systems with a higher priority go towards the beginning of the set.
            return lhs->priority > rhs->priority;
        }
    };
}