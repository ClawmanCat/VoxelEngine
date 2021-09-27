#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
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
        {}


        template <typename Component> Component& set(Component&& component) {
            return get_registry().set_component(id, fwd(component));
        }


        template <typename Component> Component remove(void) {
            return get_registry().template remove_component<Component>(id);
        }


        template <typename Component> bool has(void) const {
            return get_registry().template has_component<Component>(id);
        }


        class registry& get_registry(void) const {
            // In the future there will probably be some lookup table for registries,
            // so we don't have to waste 8 bytes on a pointer.
            return *registry;
        }


        VE_GET_VAL(id);
    private:
        entt::entity id;
        class registry* registry;
    };
}