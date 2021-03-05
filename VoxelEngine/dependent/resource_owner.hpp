#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/dependent/actor.hpp>
#include <VoxelEngine/dependent/resource_owner_registry.hpp>

#include <algorithm>


// For methods that require an actor_id to indicate the owner of some resource,
// automatically set the owner id when the game or the engine is being built
// but not otherwise. (e.g. when building a plugin.)
#if defined(VE_BUILD_ENGINE)
    #define ve_default_actor(param) ve::actor_id param = ve::engine_actor_id
#elif defined(VE_BUILD_GAME)
    #define ve_default_actor(param) ve::actor_id param = ve::game_actor_id
#else
    #define ve_default_actor(param) ve::actor_id param
#endif


namespace ve {
    // A resource owner allows an actor to store some resource in it.
    // It must automatically destroy the resource if the actor is destroyed.
    template <typename Derived> class resource_owner {
    public:
        resource_owner(void) {
            resource_owner_registry::instance().add_owner(*this);
        }
        
        resource_owner(const resource_owner&) : resource_owner() {}
        resource_owner& operator=(const resource_owner&) = default;
        
        resource_owner(resource_owner&& other) {
            resource_owner_registry::instance().transfer_owner(other, *this);
        }
        
        resource_owner& operator=(resource_owner&& other) {
            resource_owner_registry::instance().transfer_owner(other, *this);
            return *this;
        }
        
        ~resource_owner(void) {
            resource_owner_registry::instance().remove_owner(*this);
        }
        
        
        void on_actor_destroyed(actor_id id) {
            VE_CRTP_CHECK(Derived, on_actor_destroyed);
            static_cast<Derived*>(this)->on_actor_destroyed(id);
        }
        
        
        void on_actor_created(actor_id id) {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, on_actor_created)) {
                static_cast<Derived*>(this)->on_actor_created(id);
            }
        }
    };
    
    
    // Helper function for easily removing all objects owned by some actor from vector-like storage.
    // Note: this includes types with maplike interfaces that use vectors internally, like flat_map.
    template <typename Storage, typename GetOwner, typename GetName>
    inline void remove_all_owned_from_vector(
        actor_id id,
        Storage& storage,
        GetOwner get_owner,
        GetName get_name,
        std::string_view type_name
    ) {
        auto removed_begin = std::remove_if(
            storage.begin(),
            storage.end(),
            [&](const auto& elem) { return get_owner(elem) == id; }
        );
    
        VE_DEBUG_ONLY(
            for (const auto& value : ranges::subrange(removed_begin, storage.end())) {
                VE_LOG_DEBUG("Removing "s + type_name + " " + get_name(value) + " because its owner was destroyed.");
            }
        );
    
        storage.erase(removed_begin, storage.end());
    }
}