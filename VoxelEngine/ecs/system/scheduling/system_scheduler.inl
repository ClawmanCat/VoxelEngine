#pragma once


namespace ve::ecs::schedule {
    /**
     * Adds a system to the scheduler and assigns it an unique ID.
     * @param system The system to add to the scheduler.
     * @param registry The registry this scheduler is part of.
     * @return The ID of the newly added system.
     */
    template <typename Registry, typename System> inline system_id system_scheduler::add_system(System system, Registry* registry) {
        using traits = typename System::system_traits::ordering_specification;
        VE_DEBUG_ONLY(assert_main_thread());


        auto component_access = [] <typename... R, typename... W, typename... A> (meta::pack<R...>, meta::pack<W...>, meta::pack<A...>) {
            return component_access_map {
                typename component_access_map::value_type { type_index<R>(), access_mode::READ  }...,
                typename component_access_map::value_type { type_index<W>(), access_mode::WRITE }...,
                typename component_access_map::value_type { type_index<A>(), access_mode::WRITE }...
            };
        } (typename traits::read_components{}, typename traits::write_components{}, typename traits::add_remove_components{});


        auto sequence_tags = traits::sequencing_tags::apply([] <typename... Tags> {
            return std::vector<type_index_t> { type_index<Tags>()... };
        });


        const auto& data = systems.emplace(
            next_id,
            system_data {
                .system               = make_unique<system_invoke_wrapper<System, Registry>>(std::move(system), registry),
                .id                   = next_id,
                .component_access     = std::move(component_access),
                .entity_access        = traits::modifies_entities || traits::add_remove_entities ? access_mode::WRITE : access_mode::READ,
                .tags                 = std::move(sequence_tags),
                .require_exclusive    = traits::requires_exclusive,
                .require_main_thread  = traits::requires_main_thread,
                .rebuild_dependencies = [] (system_scheduler* parent, system_data& self) { parent->rebuild_dependencies_for<System>(self); }
            }
        ).first->second;


        for (const auto& tag : data.tags) system_tags[tag].push_back(data.id);
        systems_changed = true;

        // Note: on_added could invoke this method again so make sure to increment the ID beforehand.
        auto id = next_id++;
        data.system->on_added();
        return id;
    }



    /**
     * Removes the system from the scheduler and returns it as an std::optional.
     * If a system with the given ID exists, but it is of the wrong type, it is not removed.
     * @tparam System The type of the system to remove.
     * @param system The ID of the system to remove.
     * @return An std::optional containing the system if a system with the given type and ID existed, or std::nullopt otherwise.
     */
    template <typename System> inline std::optional<System> system_scheduler::take_system(system_id system) {
        VE_DEBUG_ONLY(assert_main_thread());

        if (auto it = systems.find(system); it != systems.end()) {
            if (it->second.system->get_type() != type_index<System>()) return std::nullopt;

            auto  node = systems.extract(it);
            auto& sys  = node.mapped().system;

            systems_changed = true;

            sys->on_removed();
            return std::move(*((System*) sys->get_system()));
        } else return std::nullopt;
    }



    /**
     * Returns a pointer to the system with the given ID, if a system with the given ID and type exists.
     * @tparam System The type of the system.
     * @param system The ID of the system.
     * @return A pointer to the system if it exists, or nullptr.
     */
    template <typename System> [[nodiscard]] inline System* system_scheduler::get_system(system_id system) {
        if (auto it = systems.find(system); it != systems.end()) {
            auto& wrapped_system = *(it->second.system);

            if (wrapped_system.get_type() == type_index<System>()) {
                return (System*) wrapped_system.get_system();
            } else return nullptr;
        }

        return nullptr;
    }



    /** @copydoc get_system */
    template <typename System> [[nodiscard]] inline const System* system_scheduler::get_system(system_id system) const {
        if (auto it = systems.find(system); it != systems.end()) {
            const auto& wrapped_system = *(it->second.system);

            if (wrapped_system.get_type() == type_index<System>()) {
                return *((const System*) wrapped_system.get_system());
            } else return nullptr;
        }

        return nullptr;
    }



    /**
     * Updates the set of dependencies, dependents and blacklist for the given system.
     * @tparam System The type of the system.
     * @param self The stored data for the system.
     */
    template <typename System> inline void system_scheduler::rebuild_dependencies_for(system_data& self) {
        using traits = typename System::system_traits::ordering_specification;

        traits::run_before::foreach([&] <typename Tag> {
            for (const auto id : system_tags[type_index<Tag>()]) {
                systems[id].dependencies.push_back(&self);
                self.dependents.insert(&systems[id]);
            }
        });

        traits::run_after::foreach([&] <typename Tag> {
            for (const auto id : system_tags[type_index<Tag>()]) {
                systems[id].dependents.push_back(std::addressof(self));
                self.dependencies.insert(&systems[id]);
            }
        });

        traits::run_not_during::foreach([&] <typename Tag> {
            for (const auto id : system_tags[type_index<Tag>()]) {
                systems[id].blacklist.push_back(std::addressof(self));
                self.blacklist.insert(&systems[id]);
            }
        });
    }
}