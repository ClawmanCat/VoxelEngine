#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view_type.hpp>
#include <VoxelEngine/utility/traits/nest.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>

#include <entt/entt.hpp>


namespace ve {
    namespace detail {
        template <typename Component> using component_storage_base = nest<
            entt::sigh_storage_mixin,
            entt::storage_adapter_mixin,
            entt::storage,
            Component
        >;
    }
    
    
    // Allows grouping entities at runtime, e.g. grouping entities by the chunk they're in.
    // E.g.: for (auto e : some_chunk.view() | registry.view<Components...>) { ... }
    class entity_group : public detail::component_storage_base<meta::null_type> {
    public:
        explicit entity_group(entt::registry& registry) : registry(&registry) {}
        
        
        void insert(entt::entity entity) {
            emplace(*registry, entity, meta::null_type { });
        }
        
        void remove(entt::entity entity) {
            remove(*registry, entity);
        }
        
        void transfer_to(entt::entity entity, entity_group& dest) {
            remove(entity);
            dest.insert(entity);
        }
        
        
        auto view(void) {
            return view_type<meta::pack<meta::null_type>, meta::pack<>> { *this };
        }
    
        auto view(void) const {
            return view_type<meta::pack<meta::null_type>, meta::pack<>> { *this };
        }
        
    protected:
        entt::registry* registry;
    };
    
    
    
    // Same as above, but keeps a component within the registry indicating which group an entity is in.
    // E.g.: registry.get<chunk::tracker>(e) gets a pointer to the chunk containing 'e'.
    template <typename Derived> class tracked_entity_group : public entity_group {
        using base = entity_group;
        using base::base;
        
        
        struct tracker {
            Derived* value;
            ve_dereference_as(*value);
        };
        
        
        void insert(entt::entity entity) {
            base::registry->emplace<tracker>(entity, this);
            base::insert(entity);
        }
    
        void remove(entt::entity entity) {
            base::registry->remove<tracker>(entity);
            base::remove(entity);
        }
    
        void transfer_to(entt::entity entity, entity_group<Derived>& dest) {
            base::registry->get<tracker>(entity).value = (Derived*) &dest;
            base::transfer_to(entity, dest);
        }
    };
}