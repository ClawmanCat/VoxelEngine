#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/thread/dummy_mutex.hpp>
#include <VoxelEngine/event/dispatcher.hpp>

#include <ctti/type_id.hpp>

#include <mutex>


namespace ve {
    // An event handler that dispatches events immediately as they are received.
    // The event handler can optionally be made threadsafe and cancellable. If the dispatcher is made cancellable,
    // event handlers should return true if the event is considered handled and should not be dispatched further.
    // Recursive usage of the event handler is allowed: event handlers may themselves dispatch new events,
    // and may add or remove event handlers.
    // If a new event is recursively received while another event is being dispatched, the new event is completed first.
    // If event handlers are added or removed while one or more events are being dispatched, this addition or removal
    // is processed after all aforementioned events have finished being dispatched.
    template <bool Threadsafe = false, bool Cancellable = false, typename Priority = u16>
    class simple_event_dispatcher : public dispatcher<
        simple_event_dispatcher<Threadsafe, Cancellable, Priority>,
        default_handler_for<Cancellable>::template type,
        Priority
    > {
    public:
        using simple_event_dispatcher_tag = void;
        using base_t        = typename simple_event_dispatcher::dispatcher_base_t;
        using lock_t        = std::conditional_t<Threadsafe, std::recursive_mutex, dummy_mutex>;
        using self_t        = typename base_t::self_t;
        using raw_handler   = typename base_t::raw_handler;
        using handler_token = typename base_t::handler_token;
        using priority_t    = typename base_t::priority_t;

        template <typename Event> using handler_t = typename base_t::template handler_t<Event>;

        constexpr static bool is_cancellable = Cancellable;
        constexpr static bool is_threadsafe  = Threadsafe;


        simple_event_dispatcher(void) = default;

        
        template <typename Event>
        [[nodiscard]] handler_token add_handler(handler_t<Event> handler, Priority p = Priority(0)) {
            std::lock_guard lock { mtx };


            if (is_dispatching(ctti::unnamed_type_id<Event>())) [[unlikely]] {
                // Cannot use bind_front here due to mutability.
                pending_actions.emplace_back([this, handler = std::move(handler), p, id = next_id] () mutable {
                    add_handler_impl<Event>(std::move(handler), p, id);
                });
            } else {
                add_handler_impl<Event>(std::move(handler), p, next_id);
            }


            return handler_token { meta::type_wrapper<Event>{}, next_id++, this };
        }


        template <typename Event>
        [[nodiscard]] handler_token add_one_time_handler(handler_t<Event> handler, Priority p = Priority(0)) {
            std::lock_guard lock { mtx };

            auto wrapping_handler = [this, id = next_id, handler = std::move(handler)] (const Event& event) mutable {
                handler(event);
                remove_handler<Event>(id);
            };

            return add_handler<Event>(std::move(wrapping_handler), p);
        }
        
        
        template <typename Event> void remove_handler(raw_handler id) {
            std::lock_guard lock { mtx };


            auto remove = [this, id] {
                if (auto it = handlers.find(ctti::unnamed_type_id<Event>()); it != handlers.end()) {
                    it->second->erase(id);
                }
            };


            if (is_dispatching(ctti::unnamed_type_id<Event>())) [[unlikely]] {
                pending_actions.emplace_back(std::move(remove));
            } else {
                remove();
            }
        }

    
        template <typename Event>
        void dispatch_event(const Event& event) {
            std::lock_guard lock { mtx };

            constexpr auto type = ctti::unnamed_type_id<Event>();


            // Keep track of events being dispatched to we don't edit storage while iterating
            // if the handler itself interacts with the dispatcher.
            currently_dispatched.push_back(type);

            if (auto it = handlers.find(type); it != handlers.end()) {
                it->second->invoke(&event);
            }

            currently_dispatched.pop_back();


            // Execute any actions we couldn't perform during handling of the event.
            if (currently_dispatched.empty() && !pending_actions.empty()) [[unlikely]] {
                for (const auto& action : pending_actions) action();
                pending_actions.clear();
            }
        }


        template <typename Event> bool has_handlers_for(void) {
            std::lock_guard lock { mtx };

            // Cannot check, assume there are handlers.
            if (!pending_actions.empty()) return true;

            if (auto it = handlers.find(ctti::type_id<Event>()); it != handlers.end()) {
                return !it->second->empty();
            }

            return false;
        }


        bool has_pending_events(void) const {
            return false;
        }
    private:
        template <typename Event>
        void add_handler_impl(handler_t<Event>&& handler, Priority p, raw_handler id) {
            std::lock_guard lock { mtx };


            constexpr auto type = ctti::unnamed_type_id<Event>();

            auto it = handlers.find(type);
            if (it == handlers.end()) {
                std::tie(it, std::ignore) = handlers.emplace(
                    type,
                    make_unique<handler_data<Event>>()
                );
            }

            auto& data = *((handler_data<Event>*) it->second.get());
            data.handlers[p].emplace_back(id, std::move(handler));
        }


        bool is_dispatching(ctti::type_index type) const {
            if (!currently_dispatched.empty()) [[unlikely]] {
                if (auto it = ranges::find(currently_dispatched, type); it != currently_dispatched.end()) {
                    return true;
                }
            }

            return false;
        }


        struct handler_data_base {
            virtual ~handler_data_base(void) = default;
            virtual void invoke(const void* event) = 0;
            virtual void erase(raw_handler id) = 0;
            virtual bool empty(void) const = 0;
        };
    
        template <typename Event> struct handler_data : handler_data_base {
            vec_map<Priority, std::vector<std::pair<raw_handler, handler_t<Event>>>> handlers;
            
            void invoke(const void* event) override {
                for (auto& [p, handlers_for_p] : handlers | views::reverse) {
                    for (auto& [id, handler] : handlers_for_p) {
                        if constexpr (Cancellable) {
                            if (std::invoke(handler, *((const Event*) event))) return;
                        } else {
                            std::invoke(handler, *((const Event*) event));
                        }
                    }
                }
            }
            
            void erase(raw_handler id) override {
                auto check = [&](Priority p) {
                    if (auto it = handlers.find(p); it != handlers.end()) {
                        auto& handlers_for_p = it->second;
                        
                        for (auto elem_it = handlers_for_p.begin(); elem_it != handlers_for_p.end(); ++elem_it) {
                            if (elem_it->first == id) {
                                handlers_for_p.erase(elem_it);
                                return true;
                            }
                        }
                    }
                    
                    return false;
                };
                
                
                // Since 0 is the default priority, it is worth checking that one first,
                // since it is the most likely one.
                if (check(0)) return;
                
                for (Priority p : handlers | views::keys | views::remove(0)) {
                    if (check(p)) return;
                }
            }

            bool empty(void) const override {
                return handlers.empty();
            }
        };
        
        
        hash_map<ctti::type_index, unique<handler_data_base>> handlers;
        raw_handler next_id = 0;
        mutable lock_t mtx;

        std::vector<ctti::type_index> currently_dispatched;
        std::vector<std::function<void(void)>> pending_actions;
    };
}