#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event_dispatcher.hpp>
#include <VoxelEngine/utility/utility.hpp>


namespace ve {
    template <bool AllowEventCancelling = true>
    class delayed_event_dispatcher : private event_dispatcher<AllowEventCancelling> {
        using base          = event_dispatcher<AllowEventCancelling>;
        using handler_ret_t = typename base::handler_t::return_t;
        
    public:
        using base::add_handler;
        using base::remove_handler;
        
        
        void on_actor_destroyed(actor_id id) {
            for (auto it = handleables.begin(); it != handleables.end(); ++it) {
                if (it->owner == id) swap_erase(handleables, it);
            }
            
            base::on_actor_destroyed(id);
        }
        
        
        template <typename Event> requires std::is_base_of_v<event, Event>
        void dispatch_event(Event&& evnt, ve_default_actor(owner)) {
            handleables.push_back(handleable {
                .dispatch_fn = [evnt = std::forward<Event>(evnt)](delayed_event_dispatcher* self) {
                    ((base*) self)->dispatch_event(evnt);
                },
                .owner = owner
            });
        }
        
    
        template <typename Event> requires std::is_base_of_v<event, Event>
        void dispatch_event(const Event& evnt, ve_default_actor(owner)) {
            dispatch_event(Event { evnt }, owner);
        }
        
        
        void dispatch_all(void) {
            for (const auto& handleable : handleables) {
                handleable.dispatch_fn(this);
            }
            
            handleables.clear();
        }
        
    private:
        struct handleable {
            std::function<void(delayed_event_dispatcher*)> dispatch_fn;
            actor_id owner;
        };
        
        std::vector<handleable> handleables;
    };
    
    
    using cancellable_delayed_event_dispatcher    = delayed_event_dispatcher<true>;
    using noncancellable_delayed_event_dispatcher = delayed_event_dispatcher<false>;
}