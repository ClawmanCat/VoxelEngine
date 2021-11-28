#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/component/self_component.hpp>
#include <VoxelEngine/utility/assert.hpp>

#include <entt/entt.hpp>


namespace ve {
    namespace static_entity_flags {
        // Indicates that the entity was constructed from another one, so use the components from that entity,
        // instead of initializing new ones. This is used by VE_COMPONENT initializers in derived classes.
        constexpr inline u8 move_constructed = 0b0000'0001;
    }


    // While entities need not be associated with any class, it is often useful to create an entity class
    // as a template for many identical entities.
    // By extending this base class, a class may add static components. These components are present on all entities
    // created from that class, and can be accessed as if they were class members.
    class static_entity : private registry_access_for_static_entity<static_entity> {
    public:
        using static_entity_tag = void;


        explicit static_entity(registry& registry) :
            id(registry.create_entity()),
            registry(&registry)
        {
            set(self_component { this });
        }

        ~static_entity(void) {
            // - If the entity is not owned by a static_entity_storage, we don't need to remove it from there.
            // - If the entity *is* owned by a static_entity_storage, the only way to destroy it is through there,
            //   so we don't need to ever manually remove it from there.
            if (id != entt::null) {
                get_registry().destroy_entity(id);
                id = entt::null;
            }
        }


        static_entity(static_entity&& other) { (*this) = std::move(other); }

        static_entity& operator=(static_entity&& other) {
            if (id != entt::null) {
                on_static_entity_destroyed(get_registry(), id);
                get_registry().destroy_entity(id);
            }

            registry = other.registry;
            id       = other.id;
            flags    = other.flags | static_entity_flags::move_constructed;

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
        VE_GET_VAL(flags);
    private:
        entt::entity id = entt::null;
        class registry* registry = nullptr;

        u8 flags = 0;
    };


    template <typename T> requires std::is_base_of_v<static_entity, T>
    inline T& get_self(entt::entity id, registry& registry) {
        return *((T*) registry.template get_component<self_component>(id).self);
    }
}