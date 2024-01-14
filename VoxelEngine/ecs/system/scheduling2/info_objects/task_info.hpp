#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/container/set_adapter.hpp>
#include <VoxelEngine/utility/container/map_adapter.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/scheduling2/access_mode.hpp>
#include <VoxelEngine/ecs/system/scheduling2/task_polymorphic_wrapper.hpp>
#include <VoxelEngine/ecs/system/scheduling2/info_objects/typedefs.hpp>


namespace ve::ecs::schedule {
    /** Information about a given task within the scheduler. */
    template <typename Registry> struct task_info {
        /** Polymorphic wrapper containing the underlying system. */
        unique<task_polymorphic_wrapper<Registry>> task;
        /** ID of the system stored in 'task'. */
        system_id id;

        /** Lists of dependencies and dependents of this task and list of tasks that cannot be run concurrently to this one. */
        set_adapter<const task_info*> dependencies, dependents, blacklisted;
        /** If true, this task can only be run on the main thread. */
        bool requires_main_thread;
        /** If true, no other tasks can be run concurrently to this one. */
        bool requires_exclusive_access;

        /** Sequence tags control what other systems are dependencies or dependents of this system (See @ref system_traits). */
        std::vector<sequence_tag> sequence_tags, sequence_before, sequence_after, sequence_blacklist;

        /** For every component accessed by this system, whether it is only read or also written. */
        map_adapter<component_type, access_mode> component_access;
        /** How this system accesses the set of all entities. */
        access_mode entity_access;

        /** Average time required to run this system. */
        time::duration performance;


        /** Construct a new task_info for the given [system, id] pair using the traits of the given system. */
        template <ecs_system System> static task_info from_traits(System system, system_id id) {
            using traits = typename System::system_traits;
            using sos    = typename System::system_traits::ordering_specification;


            auto to_vector = [] <typename... Tags> (meta::pack<Tags...>) {
                 return std::vector { type_id<Tags>()... };
            };

            auto to_component_map = [] <typename... R, typename... W, typename... A> (meta::pack<R...>, meta::pack<W...>, meta::pack<A...>) {
                map_adapter<component_type, access_mode> map;

                (map.insert({ type_id<A>(), access_mode::WRITE }), ...);
                (map.insert({ type_id<W>(), access_mode::WRITE }), ...);
                (map.insert({ type_id<R>(), access_mode::READ  }), ...);

                return map;
            };


            return task_info {
                .task                      = make_unique<task_polymorphic_wrapper<Registry>>(std::move(system)),
                .id                        = id,
                .requires_main_thread      = sos::requires_main_thread,
                .requires_exclusive_access = sos::requires_exclusive,
                .sequence_tags             = to_vector(typename sos::sequencing_tags {}),
                .sequence_before           = to_vector(typename sos::run_before {}),
                .sequence_after            = to_vector(typename sos::run_after {}),
                .sequence_blacklist        = to_vector(typename sos::run_not_during {}),
                .component_access          = to_component_map(typename sos::read_components {}, typename sos::write_components {}, typename sos::add_remove_components {}),
                .entity_access             = sos::modifies_entities || sos::add_remove_entities ? access_mode::WRITE : access_mode::READ,
                .performance               = time::duration { 1 }
            };
        }
    };
}