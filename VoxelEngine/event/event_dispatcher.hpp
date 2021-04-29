#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/utility/priority.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/dependent/actor.hpp>

#include <type_traits>


namespace ve {
    template <bool AllowEventCancelling = true>
    class event_dispatcher : public resource_owner<event_dispatcher<AllowEventCancelling>> {
    public:
        using default_event_dispatcher_tag = void;
        
        using handler_t = event_handler_base<AllowEventCancelling>;
        
        
        void on_actor_destroyed(actor_id id) {
            auto it = owners.find(id);
            if (it == owners.end()) return;
            
            for (const auto& handler_id : it->second) {
                auto lookup_it = handler_lookup.find(handler_id);
                if (lookup_it == handler_lookup.end()) continue;
                
                const auto& lookup = lookup_it->second;
                handlers[lookup.type][lookup.priority].erase(handler_t::dummy(handler_id));
                
                handler_lookup.erase(lookup_it);
            }
            
            owners.erase(it);
        }
        
        
        template <typename Event> requires std::is_base_of_v<event, Event>
        event_handler_id add_handler(handler_t&& handler, ve_default_actor(owner)) {
            const auto type     = ctti::type_id<Event>();
            const auto priority = handler.get_priority();
            const auto id       = handler.get_id();
            
            handler_lookup.insert({ id, handler_lookup_info { type, priority, owner } });
            owners[owner].insert(id);
            
            handlers[type][priority].insert(std::forward<handler_t>(handler));
            
            return id;
        }
    
    
        template <typename Event> requires std::is_base_of_v<event, Event>
        event_handler_id add_handler(const handler_t& handler, ve_default_actor(owner)) {
            return add_handler(copy(handler), owner);
        }
        
        
        template <typename Event> requires std::is_base_of_v<event, Event>
        void remove_handler(event_handler_id id) {
            auto lookup_it = handler_lookup.find(id);
            if (lookup_it == handler_lookup.end()) return;
            
            const auto& lookup = lookup_it->second;
            handlers[lookup.type][lookup.priority].erase(handler_t::dummy(id));
            owners[lookup.owner].erase(id);
        }
        
        
        template <typename Event> requires std::is_base_of_v<event, Event>
        void dispatch_event(const Event& evnt) {
            auto handler_storage_it = handlers.find(ctti::type_id<Event>());
            if (handler_storage_it == handlers.end()) return;
            
            for (const auto& [priority, handlers_for_event] : handler_storage_it->second | views::reverse) {
                for (const auto& handler : handlers_for_event) {
                    if constexpr (AllowEventCancelling) {
                        if (handler(evnt)) return;  // Handler has marked the event as cancelled.
                    } else {
                        handler(evnt);
                    }
                }
            }
        }
    
    private:
        // Assume few priority levels.
        // TODO: Consider faster iterating hash set (tsl?).
        using event_handler_storage = flat_map<priority, hash_set<handler_t>>;
        // For mapping ID => handler.
        struct handler_lookup_info { ctti::type_id_t type; priority priority; actor_id owner; };
        
        hash_map<ctti::type_id_t, event_handler_storage> handlers;
        hash_map<event_handler_id, handler_lookup_info> handler_lookup;
        hash_map<actor_id, hash_set<event_handler_id>> owners;
    };
    
    
    using cancellable_event_dispatcher    = event_dispatcher<true>;
    using noncancellable_event_dispatcher = event_dispatcher<false>;
};