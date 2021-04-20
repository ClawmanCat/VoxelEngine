#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/actor.hpp>
#include <VoxelEngine/utility/logger.hpp>

#include <ctti/type_id.hpp>


namespace ve {
    template <typename D> class resource_owner;
    
    
    // Store a list of all resource owners so they can be called when an actor is destroyed.
    class resource_owner_registry {
    public:
        static resource_owner_registry& instance(void);
        
        
        template <typename D> void add_owner(resource_owner<D>& owner) {
            #ifndef VE_QUIET_RES_OWNER
                VE_LOG_DEBUG("Registered new resource owner of type "s + ctti::nameof<D>().cppstring());
            #endif
            
            callbacks.insert({
                (void*) &owner,
                get_callbacks<D>()
            });
        }
        
        
        template <typename D> void remove_owner(resource_owner<D>& owner) {
            #ifndef VE_QUIET_RES_OWNER
                VE_LOG_DEBUG("Removed resource owner of type "s + ctti::nameof<D>().cppstring());
            #endif
            
            callbacks.erase((void*) &owner);
        }
        
        
        template <typename D> void transfer_owner(resource_owner<D>& from, resource_owner<D>& to) {
            auto node = callbacks.extract((void*) &from);
            node.key() = (void*) &to;
            callbacks.insert(std::move(node));
        }
        
        
        void init_actor_resources(actor_id id) {
            for (const auto& [self, callbacks] : callbacks) {
                callbacks.on_created(self, id);
            }
        }
        
        
        void unload_actor_resources(actor_id id) {
            for (const auto& [self, callbacks] : callbacks) {
                callbacks.on_destroyed(self, id);
            }
        }
    
    private:
        resource_owner_registry(void) = default;
        ve_immovable(resource_owner_registry);
        
        
        struct owner_callbacks {
            Fn<void, void*, actor_id> on_created;
            Fn<void, void*, actor_id> on_destroyed;
        };
        
        
        hash_map<void*, owner_callbacks> callbacks;
        
        
        template <typename D>
        static owner_callbacks get_callbacks(void) {
            return owner_callbacks {
                .on_created = [](void* self, actor_id id) {
                    static_cast<resource_owner<D>*>(self)->on_actor_created(id);
                },
                
                .on_destroyed = [](void* self, actor_id id) {
                    static_cast<resource_owner<D>*>(self)->on_actor_destroyed(id);
                }
            };
        }
    };
}