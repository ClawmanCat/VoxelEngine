#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/utility/heterogeneous_key.hpp>

#include <entt/entt.hpp>


namespace ve {
    class registry {
    public:
        using system_id = u32;


        registry(void) = default;
        ve_immovable(registry);


        void update(nanoseconds dt) {
            for (auto& [priority, systems] : systems_by_priority | views::reverse) {
                for (auto& [id, system] : systems) system->update(*this, dt);
            }
        }


        // System Methods
        template <typename System> requires requires { typename System::ecs_system_tag; }
        std::pair<system_id, System&> add_system(System&& system) {
            u16 priority = system.get_priority();


            auto [it, success] = systems.emplace(
                next_system_id,
                make_unique<system_data<System>>(std::move(system), priority)
            );

            systems_by_priority[priority].emplace(next_system_id, it->second.get());


            return { next_system_id++, ((unique<system_data<System>>&) it->second)->system };
        }


        void remove_system(system_id system) {
            auto it = systems.find(system);
            if (it == systems.end()) return;

            systems_by_priority[it->second->priority].erase(system);
            systems.erase(it);
        }


        // Entity Methods
        template <typename... Components>
        entt::entity create_entity(Components&&... components) {
            auto entity = storage.create();

            ([&] <typename Component> (Component&& component) {
                // Don't need to check if the component exists since this is a new entity.
                storage.template emplace<Component>(entity, fwd(component));
            }(fwd(components)), ...);

            return entity;
        }


        void destroy_entity(entt::entity entity) {
            return storage.destroy(entity);
        }


        template <typename Component> Component& set_component(entt::entity entity, Component&& component) {
            return storage.emplace_or_replace(entity, fwd(component));
        }


        template <typename Component> Component remove_component(entt::entity entity) {
            return storage.template remove<Component>(entity);
        }


        template <typename Component> bool has_component(entt::entity entity) const {
            return storage.template has<Component>(entity);
        }


        VE_GET_MREF(storage);
    private:
        entt::registry storage;


        struct system_data_base {
            explicit system_data_base(u16 priority) : priority(priority) {}

            virtual ~system_data_base(void) = default;
            virtual void update(registry& self, nanoseconds dt) = 0;

            u16 priority;
        };

        template <typename System> struct system_data : system_data_base {
            system_data(System&& system, u16 priority) : system_data_base(priority), system(std::move(system)) {}

            void update(registry& self, nanoseconds dt) override {
                system.update(System::make_view(self.storage), dt);
            }

            System system;
        };


        hash_map<system_id, unique<system_data_base>> systems;
        vec_map<u16, hash_map<system_id, system_data_base*>> systems_by_priority;

        system_id next_system_id = 0;
    };
}