#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/component/self_component.hpp>
#include <VoxelEngine/utility/assert.hpp>

#include <entt/entt.hpp>


namespace ve {
    // While entities need not be associated with any class, it is often useful to create an entity class
    // as a template for many identical entities.
    // By extending this base class, a class may add static components. These components are present on all entities
    // created from that class, and can be accessed as if they were class members.
    class static_entity {
    public:
        explicit static_entity(registry& registry) :
            id(registry.create_entity()),
            registry(&registry)
        {
            set(self_component { this });
        }

        ~static_entity(void) {
            if (id != entt::null) [[likely]] {
                get_registry().on_static_entity_destroyed(id);
                get_registry().destroy_entity(id);
            }
        }


        static_entity(static_entity&& other) { (*this) = std::move(other); }

        static_entity& operator=(static_entity&& other) {
            if (id != entt::null) {
                get_registry().on_static_entity_destroyed(id);
                get_registry().destroy_entity(id);
            }

            this->registry = other.registry;
            this->id       = other.id;

            set(self_component { this });
            other.id = entt::null;

            return *this;
        }


        // TODO: Consider allowing entity cloning through copy constructor / assignment.
        static_entity(const static_entity&) = delete;
        static_entity& operator=(const static_entity&) = delete;


        template <typename Component> Component& get(void) {
            return get_registry().template get_component<Component>(id);
        }

        template <typename Component> const Component& get(void) const {
            return get_registry().template get_component<Component>(id);
        }


        template <typename Component> Component& set(Component&& component) {
            return get_registry().template set_component<Component>(id, fwd(component));
        }

        template <typename Component> Component& set(const Component& component) {
            return get_registry().template set_component<Component>(id, component);
        }


        template <typename Component> Component remove(void) {
            return get_registry().template remove_component<Component>(id);
        }


        template <typename Component> bool has(void) const {
            return get_registry().template has_component<Component>(id);
        }


        const class registry& get_registry(void) const {
            // In the future there will probably be some lookup table for registries,
            // so we don't have to waste 8 bytes on a pointer.
            return *registry;
        }

        class registry& get_registry(void) {
            return *registry;
        }


        VE_GET_VAL(id);
    private:
        entt::entity id = entt::null;
        class registry* registry = nullptr;
    };
}