#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view_type.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>

#include <entt/entt.hpp>


namespace ve {
    namespace detail {
        template <typename Component> using component_storage_base =
            entt::sigh_storage_mixin<entt::storage_adapter_mixin<entt::storage<Component>>>;
    }
    
    
    // Allows grouping entities at runtime, e.g. grouping entities by the chunk they're in.
    // E.g.: for (auto e : some_chunk.view() | registry.view<Components...>) { ... }
    // TODO: Handle empty components like in registry.
    template <typename Data = meta::null_type>
    class storage_group : public detail::component_storage_base<Data> {
    public:
        using ecs_storage_group_tag = void;
        using storage_base          = detail::component_storage_base<Data>;


        // Default constructor is provided for the sole purpose of making this class easier to use as a class member.
        // Storage groups still need a registry in order to be usable.
        storage_group(void) = default;
        explicit storage_group(entt::registry& registry) : registry(&registry) {}
        
        
        Data& insert(entt::entity entity, auto&&... args) {
            return storage_base::emplace(*registry, entity, Data { fwd(args)... });
        }


        Data& try_insert(entt::entity entity, auto&&... args) {
            if (storage_base::contains(entity)) {
                return storage_base::get(entity);
            } else {
                return insert(entity, fwd(args)...);
            }
        }

        
        Data remove(entt::entity entity) {
            Data value = std::move(storage_base::get(entity));
            storage_base::remove(*registry, entity);
            return value;
        }


        void transfer_to(entt::entity entity, storage_group& dest) {
            Data value = remove(entity);
            dest.insert(entity, std::move(value));
        }
        
        
        auto view(void) {
            return view_type_for<meta::pack<Data>, meta::pack<>> { *this };
        }
    
        auto view(void) const {
            return view_type_for<meta::pack<const Data>, meta::pack<>> { *this };
        }


        bool valid(void) const {
            return registry;
        }
    protected:
        entt::registry* registry = nullptr;
    };
    
    
    
    // Same as above, but keeps a component within the registry indicating which group an entity is in.
    // E.g.: registry.get<chunk::tracker>(e) gets a pointer to the chunk containing 'e'.
    template <typename Derived, typename Data = meta::null_type>
    class tracked_storage_group : public storage_group<Data> {
        using base = storage_group<Data>;
        using base::base;
        
        
        struct tracker {
            Derived* value;
            ve_dereference_as(*value);
        };
        
        
        Data& insert(entt::entity entity, auto&&... args) {
            base::registry->template emplace<tracker>(entity, this);
            base::insert(entity, fwd(args)...);
        }
    
        Data remove(entt::entity entity) {
            base::registry->template remove<tracker>(entity);
            return base::remove(entity);
        }
    
        void transfer_to(entt::entity entity, tracked_storage_group<Derived, Data>& dest) {
            base::registry->template get<tracker>(entity).value = (Derived*) &dest;
            base::transfer_to(entity, dest);
        }
    };
}