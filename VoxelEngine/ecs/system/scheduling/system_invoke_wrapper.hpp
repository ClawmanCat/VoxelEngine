#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/time.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/system/scheduling/system_profiler.hpp>


namespace ve::ecs::schedule {
    /** Common base class for wrapped systems. */
    class system_invoke_base {
    public:
        explicit system_invoke_base(type_index_t type, std::string_view name) : type(type), name(name) {}
        virtual ~system_invoke_base(void) = default;

        virtual void on_added(void) = 0;
        virtual void on_removed(void) = 0;
        virtual time::duration operator()(time::duration dt, time::tick_timestamp now) = 0;

        [[nodiscard]] virtual void* get_system(void) = 0;
        [[nodiscard]] virtual const void* get_system(void) const = 0;

        VE_GET_VALS(type, name);
    private:
        type_index_t type;
        std::string_view name;
    };


    /**
     * Wrapper around a system allowing it to be invoked with a constant set of arguments and be cast to a common base.
     * @tparam System The system to wrap.
     * @tparam Registry The registry the system is part of.
     */
    template <ecs_system System, typename Registry> class system_invoke_wrapper : public system_invoke_base {
    public:
        using system_traits = typename System::system_traits;


        system_invoke_wrapper(System system, Registry* registry) :
            system_invoke_base(type_index<System>(), typename_of<System>()),
            system(std::move(system)),
            registry(registry)
        {}


        void on_added  (void) override { system.get_system().on_added(*registry);   }
        void on_removed(void) override { system.get_system().on_removed(*registry); }


        /**
         * Invokes the system and returns a performance average of how long the system took to run.
         * @param dt The amount of time simulated during this tick.
         * @param now A timestamp indicating the time of the current tick.
         * @return An averaged duration of how long the system took to run.
         */
        time::duration operator()(time::duration dt, time::tick_timestamp now) override {
            if (system.get_system().should_invoke(prev, now)) {
                std::invoke(system, make_view(), dt, prev, now);
                prev = now;
            }

            return system.get_average();
        }


        [[nodiscard]] void* get_system(void) override { return std::addressof(system.get_system()); }
        [[nodiscard]] const void* get_system(void) const override { return std::addressof(system.get_system()); }
    private:
        system_profiler<System> system;
        Registry* registry;
        time::tick_timestamp prev;


        auto make_view(void) {
            using SCM = system_traits::viewed_components;


            if constexpr (requires { typename SCM::query; }) {
                return registry->query(typename SCM::query {});
            } else {
                if constexpr (SCM::excluded_components::empty && SCM::optional_components::empty) {
                    return SCM::included_components::apply([&] <typename... I> {
                        return registry->template view<I...>();
                    });
                } else {
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