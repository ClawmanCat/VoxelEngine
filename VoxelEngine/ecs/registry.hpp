#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/registry_helpers.hpp>

#include <entt/entt.hpp>

#include <VoxelEngine/utility/assert.hpp>
#include <ctti/nameof.hpp>


namespace ve {
    class static_entity;


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
                make_unique<detail::system_data<System>>(std::move(system), priority)
            );

            systems_by_priority[priority].emplace(next_system_id, it->second.get());


            return { next_system_id++, ((unique<detail::system_data<System>>&) it->second)->system };
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


        // For static entities, it is not required to store the instance in the registry, just to keep it alive.
        // Although it can be useful to do so, so one need not store it manually.
        template <typename Entity> requires std::is_base_of_v<static_entity, Entity>
        Entity& store_static_entity(Entity&& entity) {
            entt::entity id = entity.get_id();
            auto [it, success] = static_entities.emplace(
                id,
                make_unique<detail::static_entity_storage<Entity>>(std::move(entity))
            );

            return ((detail::static_entity_storage<Entity>*) it->second.get())->entity;
        }


        void destroy_entity(entt::entity entity) {
            return storage.destroy(entity);
        }


        template <typename Component> Component& get_component(entt::entity entity) {
            return storage.template get<Component>(entity);
        }

        template <typename Component> const Component& get_component(entt::entity entity) const {
            return storage.template get<Component>(entity);
        }


        template <typename Component> requires (!std::is_reference_v<Component>) // Don't allow universal references here.
        Component& set_component(entt::entity entity, Component&& component) {
            return storage.template emplace_or_replace<Component>(entity, fwd(component));
        }

        template <typename Component> Component& set_component(entt::entity entity, const Component& component) {
            return storage.template emplace_or_replace<Component>(entity, fwd(component));
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
        // TODO: Better cache locality would probably help here, since a system is likely to iterate over many of the same type of static entity in order.
        // Consider using entt::storage with ve::stack_polymorph instead (with a fallback for very large entities), or store entities on a per-type basis.
        hash_map<entt::entity, unique<detail::static_entity_storage_base>> static_entities;


        hash_map<system_id, unique<detail::system_data_base>> systems;
        vec_map<u16, hash_map<system_id, detail::system_data_base*>> systems_by_priority;

        system_id next_system_id = 0;


        friend class static_entity;

        void on_static_entity_destroyed(entt::entity entity) {
            static_entities.erase(entity);
        }
    };
}