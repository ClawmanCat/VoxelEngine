#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/change_validator.hpp>
#include <VoxelEngine/ecs/view.hpp>
#include <VoxelEngine/ecs/component_registry.hpp>
#include <VoxelEngine/ecs/registry_helpers.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/component_tags.hpp>
#include <VoxelEngine/event/simple_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>

#include <entt/entt.hpp>


// Ensure constant storage for types that require it.
template <typename Component> requires ve::component_tags::has_constant_address_v<Component>
struct entt::component_traits<Component> : entt::basic_component_traits {
    static constexpr auto in_place_delete = true;
};


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
        virtual ~registry(void) = default;

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


            // Note: init occurs after insertion to assure address remains constant until uninit is called.
            it->second->init(*this);
            return { next_system_id++, ((unique<detail::system_data<System>>&) it->second)->system };
        }


        void remove_system(system_id system) {
            auto it = systems.find(system);
            if (it == systems.end()) return;

            // Note: uninit is called before removal to assure address remains constant during the system's time in the registry.
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
            return storage.template try_get<Component>(entity);
        }

        template <typename Component> const Component* try_get_component(entt::entity entity) const {
            return storage.template try_get<Component>(entity);
        }


        template <typename... Components> std::tuple<Components*...> try_get_components(entt::entity entity) {
            return std::tuple(try_get_component<Components>(entity)...);
        }

        template <typename... Components> std::tuple<const Components*...> try_get_components(entt::entity entity) const {
            return std::tuple(try_get_component<Components>(entity)...);
        }


        template <typename Component> Component& get_component(entt::entity entity) {
            return storage.template get<Component>(entity);
        }

        template <typename Component> const Component& get_component(entt::entity entity) const {
            return storage.template get<Component>(entity);
        }


        template <typename... Components> std::tuple<Components&...> get_components(entt::entity entity) {
            return std::forward_as_tuple(get_component<Components>(entity)...);
        }

        template <typename... Components> std::tuple<const Components&...> get_components(entt::entity entity) const {
            return std::forward_as_tuple(get_component<Components>(entity)...);
        }


        template <typename Component> requires (!std::is_reference_v<Component>)
        Component& set_component(entt::entity entity, Component&& component) {
            // This is the only component method that requires registration, since if a component type is never set,
            // it could never be present in the registry, and thus could never require registration.
            autoregister_component<Component>();


            if (has_component<Component>(entity)) {
                return storage.template replace<Component>(entity, fwd(component));
            } else {
                Component& stored_component = storage.template emplace<Component>(entity, fwd(component));

                if constexpr (component_tags::has_added_callback_v<Component>) {
                    stored_component.on_component_added(*this, entity);
                }

                dispatch_event(component_created_event<Component> { this, entity, &stored_component });

                return stored_component;
            }
        }


        template <typename Component> requires (!std::is_reference_v<Component>)
        Component& set_component(entt::entity entity, const Component& component) {
            return set_component(entity, Component { component });
        }


        // Equivalent to set_component, except the change is checked by the change validator first.
        // If the change is not allowed, no changes are made to the registry.
        template <typename Component> requires (!std::is_reference_v<Component>)
        std::pair<change_result, Component*> set_component_checked(instance_id remote, entt::entity entity, Component&& component) {
            Component* value = try_get_component<Component>(entity);

            auto result = validator.is_allowed(remote, *this, entity, value, &component);
            if (result == change_result::ALLOWED) value = &set_component(entity, fwd(component));

            return { result, value };
        }


        template <typename Component> requires (!std::is_reference_v<Component>)
        std::pair<change_result, Component*> set_component_checked(instance_id remote, entt::entity entity, const Component& component) {
            return set_component_checked(remote, entity, Component { component });
        }


        template <typename Component> requires component_tags::is_removable_v<Component>
        Component remove_component(entt::entity entity) {
            Component& stored_component = get_component<Component>(entity);

            if constexpr (component_tags::has_removed_callback_v<Component>) {
                stored_component.on_component_removed(*this, entity);
            }

            dispatch_event(component_destroyed_event<Component> { this, entity, &stored_component });

            Component component = std::move(stored_component);
            storage.template remove<Component>(entity);

            return component;
        }


        // Equivalent to remove_component, except the change is checked by the change validator first.
        // If the change is not allowed, no changes are made to the registry and the returned component is the old value of the component.
        // If the change is allowed, the returned component is null.
        template <typename Component>
        std::pair<change_result, Component*> remove_component_checked(instance_id remote, entt::entity entity) {
            Component* value = try_get_component<Component>(entity);
            auto result = validator.template is_allowed<Component>(remote, *this, entity, value, nullptr);

            // If the component is non-removable, just return forbidden or unobservable.
            if constexpr (!component_tags::is_removable_v<Component>) {
                return { result == change_result::ALLOWED ? change_result::FORBIDDEN : result, value };
            } else {
                if (result == change_result::ALLOWED) {
                    remove_component<Component>(entity);
                    value = nullptr;
                }
            }

            return { result, value };
        }


        template <typename Component> requires component_tags::is_removable_v<Component>
        void remove_all_components(auto& view) {
            if constexpr (component_tags::has_removed_callback_v<Component>) {
                for (auto entity : view) {
                    const auto& stored_component = view.template get<Component>(entity);
                    stored_component.on_component_removed(*this, entity);
                }
            }

            // Skip event handling if there are no handlers.
            if (has_handlers_for<component_destroyed_event<Component>>()) {
                for (auto entity : view) {
                    const auto& stored_component = view.template get<Component>(entity);
                    dispatch_event(component_destroyed_event<Component> { this, entity, &stored_component });
                }
            }

            storage.template remove<Component>(view.begin(), view.end());
        }


        template <typename Component> requires component_tags::is_removable_v<Component>
        void remove_all_components(void) {
            auto v = view<Component>();

            if constexpr (component_tags::has_removed_callback_v<Component>) {
                for (auto entity : v) {
                    const auto& stored_component = v.template get<Component>(entity);
                    stored_component.on_component_removed(*this, entity);
                }
            }

            // Skip event handling if there are no handlers.
            if (has_handlers_for<component_destroyed_event<Component>>()) {
                for (auto entity : v) {
                    const auto& stored_component = v.template get<Component>(entity);
                    dispatch_event(component_destroyed_event<Component> { this, entity, &stored_component });
                }
            }

            storage.template clear<Component>();
        }


        template <typename Component> bool has_component(entt::entity entity) const {
            return storage.template all_of<Component>(entity);
        }

        template <typename... Components> bool has_all(entt::entity entity) const {
            return storage.template all_of<Components...>(entity);
        }

        template <typename... Components> bool has_any(entt::entity entity) const {
            return storage.template any_of<Components...>(entity);
        }


        // Views
        template <typename... Components> auto view(void) {
            return storage.template view<Components...>();
        }

        template <typename... Components> auto view(void) const {
            return storage.template view<Components...>();
        }

        template <meta::pack_of_types Required, meta::pack_of_types Excluded = meta::pack<>> auto view_pack(void) {
            return view_from_registry<Required, Excluded>(storage);
        }

        template <meta::pack_of_types Required, meta::pack_of_types Excluded = meta::pack<>> auto view_pack(void) const {
            return view_from_registry<Required, Excluded>(storage);
        }


        VE_GET_MREF(validator);
        // Note: acting upon the storage directly will cause events to not be fired, and should be avoided, as systems may depend on them.
        VE_GET_MREF(storage);
    private:
        template <typename... Components> void create_entity_common(entt::entity entity, Components&&... components) {
            dispatch_event(entity_created_event { this, entity });

            ([&] <typename Component> (Component&& component) {
                autoregister_component<Component>();

                // Don't need to check if the component exists since this is a new entity.
                Component& stored_component = storage.template emplace<Component>(entity, fwd(component));

                if constexpr (component_tags::has_added_callback_v<Component>) {
                    stored_component.on_component_added(*this, entity);
                }

                dispatch_event(component_created_event<Component> { this, entity, &stored_component });
            }(fwd(components)), ...);
        }


        change_validator validator;


        // TODO: Better cache locality would probably help here, since a system is likely to iterate over many of the same type of static entity in order.
        // Consider using entt::storage with ve::stack_polymorph instead (with a fallback for very large entities), or store entities on a per-type basis.
        hash_map<entt::entity, unique<detail::static_entity_storage_base>> static_entities;
        entt::registry storage;


        hash_map<system_id, unique<detail::system_data_base>> systems;
        vec_map<u16, hash_map<system_id, detail::system_data_base*>> systems_by_priority;

        system_id next_system_id = 0;
    };


    // Wrappers around registry functions. Unlike the registry member methods, these can be forward declared.
    namespace registry_callbacks {
        template <typename T> void set_component(registry& r, entt::entity e, T&& v) {
            r.set_component(e, fwd(v));
        }


        template <typename T> T& get_component(registry& r, entt::entity e) {
            return r.template get_component<T>(e);
        }


        template <typename T> void remove_component(registry& r, entt::entity e) {
            if constexpr (component_tags::is_removable_v<T>) r.template remove_component<T>(e);
            else VE_ASSERT(false, "Attempt to remove non-removable component");
        }


        template <typename T> std::pair<change_result, T*> set_component_checked(instance_id remote, registry& r, entt::entity e, T&& v) {
            return r.set_component_checked(remote, e, fwd(v));
        }


        template <typename T> std::pair<change_result, T*> remove_component_checked(instance_id remote, registry& r, entt::entity e) {
            return r.template remove_component_checked<T>(remote, e);
        }
    }
}