#pragma once


namespace ve::ecs::schedule {
    /** Adds the given task to the given scheduler info and updates the dependency graph. */
    template <typename R> inline void scheduler_info<R>::add_task(R* registry, task_info<R>&& task) {
        for (const auto& tag : task->sequence_tags) sequence_tags[tag].push_back(&task);


        // For every tag in the task's tag_list, add every system with that tag to the tasks self_list and add this system to the other system's other_list.
        auto update_field = [&] (auto& task, auto tag_list, auto self_list, auto other_list) {
            for (const auto& tag : task.*tag_list) {
                for (auto* ptr : sequence_tags[tag]) {
                    auto& other_task = *ptr;

                    (task.*self_list).insert(&other_task);
                    (other_task.*other_list).insert(&task);
                }
            }
        };


        update_field(task, &task_info<R>::sequence_before,    &task_info<R>::dependents,   &task_info<R>::dependencies);
        update_field(task, &task_info<R>::sequence_after,     &task_info<R>::dependencies, &task_info<R>::dependents);
        update_field(task, &task_info<R>::sequence_blacklist, &task_info<R>::blacklisted,  &task_info<R>::blacklisted);


        auto [it, success] = tasks.emplace(task.id, std::move(task));
        it->second.task->on_added(registry);
    }



    /** Removes the given task from the given scheduler info and updates the dependency graph. */
    template <typename R> inline void scheduler_info<R>::remove_task(R* registry, const task_info<R>& task) {
        task.task->on_removed(registry);


        // For every system in this task's self_list, add or remove this system from that system's other_list.
        auto update_field = [&] (auto self_list, auto other_list) {
            for (const auto* other : task.*self_list) {
                // Not very clean but the alternative would be to get the mutable version through the scheduler, which would do the exact same thing except slower.
                auto* mutable_other = const_cast<task_info<R>*>(other);
                mutable_other->*other_list.erase(&task);
            }
        };


        update_field(&task_info<R>::dependents,   &task_info<R>::dependencies);
        update_field(&task_info<R>::dependencies, &task_info<R>::dependents);
        update_field(&task_info<R>::blacklisted,  &task_info<R>::blacklisted);


        for (const auto& tag : task->sequence_tags) sequence_tags[tag].erase(&task);
        tasks.erase(task.id);
    }
}