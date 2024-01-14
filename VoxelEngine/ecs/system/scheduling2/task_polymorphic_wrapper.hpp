#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/scheduling2/task_profiling_wrapper.hpp>
#include <VoxelEngine/ecs/view/query/query_building_blocks.hpp>


namespace ve::ecs::schedule {
    /** Wrapper around profiled ECS tasks that can be cast to a common base class (interface). */
    template <typename Registry> class task_polymorphic_wrapper {
    public:
        task_polymorphic_wrapper(const type_id_t& type) : type(type) {}

        virtual ~task_polymorphic_wrapper(void) = default;

        virtual void operator()(Registry*, time::duration, time::tick_timestamp) = 0;
        virtual void on_added(Registry*) {}
        virtual void on_removed(Registry*) {}

        template <typename Task> [[nodiscard]] Task*       get_task(void)       { return (type == type_id<Task>()) ? (Task*)       get_task_pointer() : nullptr; }
        template <typename Task> [[nodiscard]] const Task* get_task(void) const { return (type == type_id<Task>()) ? (const Task*) get_task_pointer() : nullptr; }

        [[nodiscard]] std::string_view get_name(void) const { return type.name(); }

        VE_GET_VALS(type, performance);
    protected:
        [[nodiscard]] virtual void* get_task_pointer(void) = 0;
        [[nodiscard]] virtual const void* get_task_pointer(void) const = 0;

        VE_SET_VALS(performance);
    private:
        type_id_t type;
        time::duration performance;
    };


    /** Wrapper around profiled ECS tasks that can be cast to a common base class (implementation). */
    template <typename Registry, ecs_system Task> class profiled_ecs_task : public task_polymorphic_wrapper<Registry> {
    public:
        explicit profiled_ecs_task(Task task) :
            task_polymorphic_wrapper<Registry>(type_id<Task>()),
            profiled_task(std::move(task))
        {}


        void operator()(Registry* registry, time::duration dt, time::tick_timestamp now) override {
            std::invoke(profiled_task, make_view_for_task(registry), dt, now);
            set_performance(profiled_task.get_average_performance());
        }


        void on_added(Registry* registry) {
            profiled_task.get_task().on_added(*registry);
        }


        void on_removed(Registry* registry) {
            profiled_task.get_task().on_removed(*registry);
        }
    protected:
        [[nodiscard]] void* get_task_pointer(void) override {
            return std::addressof(profiled_task.get_task());
        }


        [[nodiscard]] const void* get_task_pointer(void) const override {
            return std::addressof(profiled_task.get_task());
        };

    private:
        task_profiling_wrapper<Task> profiled_task;


        /** Constructs a view of the registry for the given ECS system from its system_traits. */
        static auto make_view_for_task(Registry* registry) {
            using SCM = typename Task::system_traits::viewed_components;

            // If the system traits contain a query just forward that to the registry.
            if constexpr (requires { typename SCM::query; }) {
                return registry->query(typename SCM::query {});
            }

            // If the system traits contain type lists, try using view first since it may be more performant,
            // or construct a query from the type lists otherwise.
            else {
                // Include-list only: use view.
                if constexpr (SCM::excluded_components::empty && SCM::optional_components::empty) {
                    return SCM::included_components::apply([&] <typename... I> {
                        return registry->template view<I...>();
                    });
                }

                // Exclude- or optional-lists: construct a query.
                else {
                    return [&] <typename... I, typename... E, typename... O> (meta::pack<I...>, meta::pack<E...>, meta::pack<O...>) {
                        return registry->query(
                            ( query::has<I> && ... && query::true_query) &&
                            (!query::has<E> && ... && query::true_query) &&
                            ( query::has<O> || ... || query::true_query)
                        );
                    } (typename SCM::included_components {}, typename SCM::excluded_components {}, typename SCM::optional_components {});
                }
            }
        }
    };
}