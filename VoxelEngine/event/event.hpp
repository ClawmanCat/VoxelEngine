#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/priority.hpp>
#include <VoxelEngine/utility/thread/threadsafe_id.hpp>
#include <VoxelEngine/utility/traits/pack.hpp>

#include <functional>
#include <type_traits>


namespace ve {
    struct event {};    // Not cast to base, no virtual destructor.
    
    using event_handler_id = u64;
    
    
    template <bool Cancellable> class event_handler_base {
    public:
        // If an event is cancellable, the handler should return true if the event should be cancelled.
        using return_t  = std::conditional_t<Cancellable, bool, void>;
        using wrapped_t = std::function<return_t(const event&)>;
        
        
        // is_loosely_same check prevents this constructor from being ambiguous with the move constructor.
        template <typename Pred> requires std::is_invocable_v<Pred, const event&> && (!is_loosely_same<Pred, event_handler_base>)
        explicit event_handler_base(Pred&& pred, priority priority = priority::NORMAL) :
            handler(std::forward<Pred>(pred)),
            priority(priority),
            id(threadsafe_id<"event_handler">::next())
        {}
    
    
        return_t operator()(const event& e) const { return handler(e); }
        
        ve_move_only(event_handler_base);
        ve_comparable_fields(event_handler_base, id);
        ve_hashable(id);
        
        VE_GET_CREF(handler);
        VE_GET_VAL(priority);
        VE_GET_VAL(id);
        
        
        static event_handler_base dummy(u64 id) {
            return event_handler_base(id);
        }
    private:
        // Dummy event for heterogeneous lookup.
        event_handler_base(u64 id) : handler(nullptr), priority(priority::NORMAL), id(id) {}
        
        
        wrapped_t handler;
        priority priority;
        event_handler_id id;
    };
    
    
    using event_handler             = event_handler_base<false>;
    using cancellable_event_handler = event_handler_base<true>;
}