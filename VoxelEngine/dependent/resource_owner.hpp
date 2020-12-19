#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/actor_id.hpp>
#include <VoxelEngine/dependent/resource_owner_registry.hpp>
#include <VoxelEngine/engine.hpp>
#include <VoxelEngine/utils/crtp.hpp>



// For methods that require an actor_id to indicate the owner of some resource,
// automatically sets the owner id when the game or the engine is being built
// but not otherwise. (e.g. when building a plugin.)
#if defined(VE_BUILD_ENGINE)
    #define VE_RESOURCE_FN(name, ...) name(__VA_ARGS__, ve::actor_id owner = ve::engine_actor_id)
#elif defined(VE_BUILD_GAME)
    #define VE_RESOURCE_FN(name, ...) name(__VA_ARGS__, ve::actor_id owner = ve::game_actor_id)
#else
    #define VE_RESOURCE_FN(name, ...) name(__VA_ARGS__, ve::actor_id owner)
#endif


namespace ve {
    // A resource owner is a class that has some resource which is associated with some actor, (e.g. the engine or a plugin)
    // which must be destroyed when the actor is destroyed.
    template <typename Derived> struct resource_owner {
        resource_owner(void) {
            resource_owner_registry::instance().insert({ this, &resource_owner<Derived>::on_actor_destroyed_wrapper });
        }
        
        resource_owner(const resource_owner& o) {
            resource_owner_registry::instance().insert({ this, &resource_owner<Derived>::on_actor_destroyed_wrapper });
        }
    
        resource_owner(resource_owner&& o) {
            resource_owner_registry::instance().erase(&o);
            resource_owner_registry::instance().insert({ this, &resource_owner<Derived>::on_actor_destroyed_wrapper });
        }
        
        resource_owner& operator=(const resource_owner& o) = default;
    
        resource_owner& operator=(resource_owner&& o) {
            resource_owner_registry::instance().erase(&o);
        }
        
        ~resource_owner(void) {
            resource_owner_registry::instance().erase(this);
        }
    
    
        void on_actor_destroyed(actor_id id) {
            VE_CRTP_GUARD(on_actor_destroyed);
            static_cast<Derived*>(this)->on_actor_destroyed(id);
        }
        
        
        static void on_actor_destroyed_wrapper(void* self, actor_id id) {
            static_cast<resource_owner<Derived>*>(self)->on_actor_destroyed(id);
        }
    };
}