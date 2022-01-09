#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry_helpers.hpp>
#include <VoxelEngine/ecs/empty_storage.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>

#include <entt/entt.hpp>


namespace ve {
    class registry;
    class static_entity;


    struct entity_created_event   { registry* owner; entt::entity entity; };
    struct entity_destroyed_event { registry* owner; entt::entity entity; };
    template <typename Component> struct component_created_event   { registry* owner; entt::entity entity; const Component* component; };
    template <typename Component> struct component_destroyed_event { registry* owner; entt::entity entity; const Component* component; };


    // The registry is responsible for storing entities, components and systems.
    class registry : public subscribe_only_view<simple_event_dispatcher<false>> {
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


            it->second->init(*this);
            return { next_system_id++, ((unique<detail::system_data<System>>&) it->second)->system };
        }


        void remove_system(system_id system) {
            auto it = systems.find(system);
            if (it == systems.end()) return;

            it->second->uninit(*this);

            systems_by_priority[it->second->priority].erase(system);
            systems.erase(it);
        }


        // Entity Methods
        template <typename... Components>
        entt::entity create_entity(Components&&... components) {
            auto entity = storage.create();
            create_entity_common(entity, fwd(components)...);

            return entity;
        }


        template <typename... Components>
        entt::entity create_entity_with_id(entt::entity id, Components&&... components) {
            auto entity = storage.create(id);
            create_entity_common(entity, fwd(components)...);

            return entity;
        }


        // It is allowed, but not required, to store a static entity within the registry containing its components.
        // If this is done, the static entity will be automatically destroyed when the underlying entity is destroyed.
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
            dispatch_event(entity_destroyed_event { this, entity });

            if (auto it = static_entities.find(entity); it != static_entities.end()) {
                static_entities.erase(it);
            }

            storage.destroy(entity);
        }


        template <typename Component> entt::entity entity_for_component(const Component& cmp) const {
            return entt::to_entity(storage, cmp);
        }


        // Component Methods
        template <typename Component> Component* try_get_component(entt::entity entity) {
            if constexpr (std::is_empty_v<Component>) return &empty_storage_for<Component>();
            else return storage.template try_get<Component>(entity);
        }

        template <typename Component> const Component* try_get_component(entt::entity entity) const {
            if constexpr (std::is_empty_v<Component>) return &empty_storage_for<Component>();
            else return storage.template try_get<Component>(entity);
        }


        template <typename Component> Component& get_component(entt::entity entity) {
            if constexpr (std::is_empty_v<Component>) return empty_storage_for<Component>();
            else return storage.template get<Component>(entity);
        }

        template <typename Component> const Component& get_component(entt::entity entity) const {
            if constexpr (std::is_empty_v<Component>) return empty_storage_for<Component>();
            else return storage.template get<Component>(entity);
        }


        template <typename Component> requires (!std::is_reference_v<Component>)
        Component& set_component(entt::entity entity, Component&& component) {
            if (has_component<Component>(entity)) {
                return ve_impl_component_access(Component, storage.template replace<Component>, entity, fwd(component));
            } else {
                Component& stored_component = ve_impl_component_access(Component, storage.template emplace<Component>, entity, fwd(component));
                dispatch_event(component_created_event<Component> { this, entity, &stored_component });

                return stored_component;
            }
        }


        template <typename Component> requires (!std::is_reference_v<Component>)
        Component& set_component(entt::entity entity, const Component& component) {
            return set_component(entity, Component { component });
        }


        template <typename Component> requires (!requires { typename Component::non_removable_tag; })
        Component remove_component(entt::entity entity) {
            Component& stored_component = get_component<Component>(entity);
            dispatch_event(component_destroyed_event<Component> { this, entity, &stored_component });

            Component component = std::move(stored_component);
            storage.template remove<Component>(entity);

            return component;
        }


        template <typename Component> requires (!requires { typename Component::non_removable_tag; })
        void remove_all_components(void) {
            auto v = view<Component>();

            // Skip event handling if there are no handlers.
            if (has_handlers_for<component_destroyed_event<Component>>()) {
                for (auto entity : v) {
                    dispatch_event(component_destroyed_event<Component> { this, entity, &v.template get<Component>(entity) });
                }
            }

            storage.template clear<Component>();
        }


        template <typename Component> bool has_component(entt::entity entity) const {
            return storage.template has<Component>(entity);
        }


        // Views
        template <typename... Components> auto view(void) {
            return construct_view<meta::pack<Components...>, meta::pack<>>(storage);
        }

        template <typename... Components> auto view(void) const {
            return construct_view<meta::pack<Components...>, meta::pack<>>(storage);
        }

        template <meta::pack_of_types Required, meta::pack_of_types Excluded> auto view_except(void) {
            return construct_view<Required, Excluded>(storage);
        }

        template <meta::pack_of_types Required, meta::pack_of_types Excluded> auto view_except(void) const {
            return construct_view<Required, Excluded>(storage);
        }


        // Note: acting upon the storage directly will cause events to not be fired, and should be avoided, as systems may depend on them.
        VE_GET_MREF(storage);
    private:
        template <typename... Components> void create_entity_common(entt::entity entity, Components&&... components) {
            set_component(entity, detail::common_component { });
            dispatch_event(entity_created_event { this, entity });

            ([&] <typename Component> (Component&& component) {
                // Don't need to check if the component exists since this is a new entity.
                Component& stored_component = ve_impl_component_access(Component, storage.template emplace, entity, fwd(component));
                dispatch_event(component_created_event<Component> { this, entity, &stored_component });
            }(fwd(components)), ...);
        }


        // TODO: Better cache locality would probably help here, since a system is likely to iterate over many of the same type of static entity in order.
        // Consider using entt::storage with ve::stack_polymorph instead (with a fallback for very large entities), or store entities on a per-type basis.
        hash_map<entt::entity, unique<detail::static_entity_storage_base>> static_entities;
        entt::registry storage;


        hash_map<system_id, unique<detail::system_data_base>> systems;
        vec_map<u16, hash_map<system_id, detail::system_data_base*>> systems_by_priority;

        system_id next_system_id = 0;
    };
}