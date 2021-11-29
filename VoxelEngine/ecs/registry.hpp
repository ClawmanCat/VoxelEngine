#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/registry_helpers.hpp>
#include <VoxelEngine/ecs/component_registry.hpp>
#include <VoxelEngine/ecs/change_validator.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>

#include <entt/entt.hpp>

#include <VoxelEngine/utility/assert.hpp>
#include <ctti/nameof.hpp>


namespace ve {
    class static_entity;
    entt::registry& detail::get_storage(registry& registry);


    template <typename Derived> struct registry_access_for_visibility_provider;
    template <typename Derived> struct registry_access_for_static_entity;


    struct entity_created_event   { entt::entity entity; };
    struct entity_destroyed_event { entt::entity entity; };
    template <typename Component> struct component_created_event   { entt::entity entity; const Component* component; };
    template <typename Component> struct component_destroyed_event { entt::entity entity; const Component* component; };


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
            dispatch_event(entity_created_event { entity });

            ([&] <typename Component> (Component&& component) {
                VE_REGISTER_COMPONENT_T(Component);

                // Don't need to check if the component exists since this is a new entity.
                Component& stored_component = storage.template emplace<Component>(entity, fwd(component));
                dispatch_event(component_created_event<Component> { entity, &stored_component });
            }(fwd(components)), ...);

            return entity;
        }


        template <typename... Components>
        entt::entity create_entity_with_id(entt::entity id, Components&&... components) {
            auto entity = storage.create(id);
            VE_ASSERT(entity == id, "Failed to create entity with ID ", id, ": an entity with this ID already exists.");

            dispatch_event(entity_created_event { entity });

            ([&] <typename Component> (Component&& component) {
                VE_REGISTER_COMPONENT_T(Component);

                // Don't need to check if the component exists since this is a new entity.
                Component& stored_component = storage.template emplace<Component>(entity, fwd(component));
                dispatch_event(component_created_event<Component> { entity, &stored_component });
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
            dispatch_event(entity_destroyed_event { entity });
            return storage.destroy(entity);
        }


        template <typename Component> Component* try_get_component(entt::entity entity) {
            return storage.template try_get<Component>(entity);
        }

        template <typename Component> const Component* try_get_component(entt::entity entity) const {
            return storage.template try_get<Component>(entity);
        }


        template <typename Component> Component& get_component(entt::entity entity) {
            return storage.template get<Component>(entity);
        }

        template <typename Component> const Component& get_component(entt::entity entity) const {
            return storage.template get<Component>(entity);
        }


        template <typename Component> requires (!std::is_reference_v<Component>) // Don't allow universal references here.
        Component& set_component(entt::entity entity, Component&& component) {
            VE_REGISTER_COMPONENT_T(Component);


            if (has_component<Component>(entity)) {
                return storage.template replace<Component>(entity, fwd(component));
            } else {
                Component& stored_component = storage.template emplace<Component>(entity, fwd(component));
                dispatch_event(component_created_event<Component> { entity, &stored_component });

                return stored_component;
            }
        }

        template <typename Component> Component& set_component(entt::entity entity, const Component& component) {
            VE_REGISTER_COMPONENT_T(Component);


            if (has_component<Component>(entity)) {
                return storage.template replace<Component>(entity, fwd(component));
            } else {
                Component& stored_component = storage.template emplace<Component>(entity, fwd(component));
                dispatch_event(component_created_event<Component> { entity, &stored_component });

                return stored_component;
            }
        }


        template <typename Component> Component remove_component(entt::entity entity) {
            Component& stored_component = get_component<Component>(entity);
            dispatch_event(component_destroyed_event<Component> { entity, &stored_component });

            Component component = std::move(stored_component);
            storage.template remove<Component>(entity);

            return component;
        }


        template <typename Component> bool has_component(entt::entity entity) const {
            return storage.template has<Component>(entity);
        }


        template <typename... Components> auto view(void) {
            return storage.template view<Components...>();
        }


        template <typename... Components> auto view(void) const {
            return storage.template view<Components...>();
        }


        // Visibility Management
        bool is_visible(instance_id remote, entt::entity entity) const {
            return visibility_provider.system
                ? visibility_provider.invoke(visibility_provider.system, remote, entity)
                : visible_by_default;
        }


        // Controls the default visibility when there is no visibility management system.
        bool get_default_visibility(void) const { return visible_by_default; }
        void set_default_visibility(bool enabled) { visible_by_default = enabled; }


        // Can't give out mutable references, since we have to ensure events are dispatched.
        VE_GET_CREF(storage);
        VE_GET_MREF(validator);
    private:
        template <typename Derived> friend struct registry_access_for_visibility_provider;
        template <typename Derived> friend struct registry_access_for_static_entity;

        // Access to storage to create views for system invocation.
        friend entt::registry& detail::get_storage(registry& registry);


        // TODO: Better cache locality would probably help here, since a system is likely to iterate over many of the same type of static entity in order.
        // Consider using entt::storage with ve::stack_polymorph instead (with a fallback for very large entities), or store entities on a per-type basis.
        hash_map<entt::entity, unique<detail::static_entity_storage_base>> static_entities;
        entt::registry storage;


        hash_map<system_id, unique<detail::system_data_base>> systems;
        vec_map<u16, hash_map<system_id, detail::system_data_base*>> systems_by_priority;

        system_id next_system_id = 0;


        change_validator validator;

        bool visible_by_default = true;
        struct {
            const void* system = nullptr;
            fn<bool, const void*, instance_id, entt::entity> invoke;
        } visibility_provider;
    };


    template <typename Derived> struct registry_access_for_visibility_provider {
    private:
        void ve_impl_assert_friend(void) {
            // Must be in a function to prevent type incompleteness.
            static_assert(requires { typename Derived::visibility_provider_tag; });
        }

    protected:
        void set_visibility_provider(registry& registry) {
            if (registry.visibility_provider.system) {
                throw std::runtime_error { "Registry may have at most one active visibility provider." };
            }

            registry.visibility_provider.system = this;
            registry.visibility_provider.invoke = [](const void* self, instance_id remote, entt::entity entity) {
                return static_cast<const Derived*>(self)->is_visible(entity, remote);
            };
        }

        void clear_visibility_provider(registry& registry) {
            registry.visibility_provider.system = nullptr;
        }
    };


    template <typename Derived> struct registry_access_for_static_entity {
    private:
        void ve_impl_assert_friend(void) {
            // Must be in a function to prevent type incompleteness.
            static_assert(requires { typename Derived::static_entity_tag; });
        }

    protected:
        void on_static_entity_destroyed(registry& registry, entt::entity entity) {
            registry.static_entities.erase(entity);
        }
    };


    // Implementations for methods forward declared in component_registry.hpp.
    namespace detail {
        // Attempts to set the component to the value stored in the provided buffer.
        // If the provided remote is not allowed to perform this action, no changes to the registry are made.
        template <typename T> inline change_result set_component(instance_id remote, registry& r, entt::entity e, std::span<const u8> data) {
            if (!r.is_visible(remote, e)) return change_result::UNOBSERVABLE;

            const T* old_value = r.template try_get_component<T>(e);
            T new_value = serialize::from_bytes<T>(data);

            if (!r.get_validator().is_allowed(remote, r, e, old_value, &new_value)) return change_result::DENIED;

            r.set_component(e, std::move(new_value));
            return change_result::ALLOWED;
        }


        // Attempts to remote the component from the registry.
        // If the provided remote is not allowed to perform this action, no changes to the registry are made.
        template <typename T> inline change_result remove_component(instance_id remote, registry& r, entt::entity e) {
            if (!r.is_visible(remote, e)) return change_result::UNOBSERVABLE;

            const T* old_value = r.template try_get_component<T>(e);

            if (!r.get_validator().is_allowed(remote, r, e, old_value, (const T*) nullptr)) return change_result::DENIED;

            r.template remove_component<T>(e);
            return change_result::ALLOWED;
        }


        // Serializes and returns the given component from the registry.
        template <typename T> inline std::vector<u8> get_component(registry& r, entt::entity e) {
            return serialize::to_bytes(r.template get_component<T>(e));
        }
    }
}