#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/debug/assert.hpp>
#include <VoxelEngine/ecs/system/scheduling2/access_mode.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/typedefs.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/task_info.hpp>


namespace ve::ecs::schedule {
    /** Tracks the ways in which the ECS is accessed by currently running tasks. */
    struct access_tracker {
        hash_map<component_type, access_counter> component_access;
        access_counter entity_access;
        bool exclusive_access_proclaimed = false;


        /** Returns true if any ECS resources are currently marked as accessed. */
        bool has_ongoing_access(void) const {
            if (exclusive_access_proclaimed) return true;
            if (entity_access.has_writer || entity_access.readers > 0) return true;

            for (const auto& [type, access] : component_access) {
                if (access.has_writer || access.readers > 0) return true;
            }

            return false;
        }


        /** Returns true if the given task requires access to the ECS which conflicts which another task that is already running. */
        template <typename R> bool has_access_conflict(const task_info<R>& task) const {
            if (exclusive_access_proclaimed) return true;
            if (!entity_access.can_add_with_mode(task.entity_access)) return true;

            for (const auto& [type, access] : task.component_access) {
                if (auto it = component_access.find(type); it != component_access.end()) {
                    if (!it->second.can_add_with_mode(access)) return true;
                }
            }

            return false;
        }


        /** Marks the ECS resources accessed by the given task as occupied. */
        template <typename R> void add_task_access(const task_info<R>& task) {
            VE_DEBUG_ASSERT(!has_access_conflict(task), "Attempt to add task which conflicts with current resource-access.");

            for (const auto& [type, access] : task.component_access) {
                component_access[type].add_with_mode(access);
            }

            entity_access.add_with_mode(task.entity_access);
            if (task.requires_exclusive_access) exclusive_access_proclaimed = true;
        }


        /** Marks the ECS resources accessed by the given task as no longer occupied. */
        template <typename R> void remove_task_access(const task_info<R>& task) {
            VE_DEBUG_ASSERT(has_access_conflict(task), "Attempt to remove access for task which was not currently accessed.");

            for (const auto& [type, access] : task.component_access) {
                component_access[type].remove_with_mode(access);
            }

            entity_access.remove_with_mode(task.entity_access);
            exclusive_access_proclaimed = false;
        }
    };
}