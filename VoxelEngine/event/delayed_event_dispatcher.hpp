#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/thread/dummy_mutex.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>
#include <VoxelEngine/utility/algorithm.hpp>
#include <VoxelEngine/event/event_handler_id.hpp>

#include <ctti/type_id.hpp>

#include <mutex>


namespace ve {
    template <
        bool Cancellable = true,
        typename Priority = u16,
        bool Threadsafe = false
    > class delayed_event_dispatcher {
    public:
        using delayed_event_dispatcher_tag = void;
        using handler_id = event_handler_id_t;
        using priority_t = Priority;
        using lock_t     = std::conditional_t<Threadsafe, std::recursive_mutex, dummy_mutex>;

        constexpr static bool is_cancellable = Cancellable;
        constexpr static bool is_threadsafe  = Threadsafe;


        delayed_event_dispatcher(void) = default;
        ve_immovable(delayed_event_dispatcher);

        
        // Cancelling handlers should return true to cancel event dispatching.
        template <typename Event>
        using handler_t = std::function<std::conditional_t<Cancellable, bool, void>(const Event&)>;
        
        
        template <typename Event>
        handler_id add_handler(handler_t<Event> handler, Priority p = Priority(0)) {
            std::lock_guard lock { mtx };

            auto action = [this, handler = std::move(handler), p, id = next_id] () mutable {
                constexpr auto type = ctti::unnamed_type_id<Event>();

                auto it = handlers.find(type);
                if (it == handlers.end()) {
                    std::tie(it, std::ignore) = handlers.emplace(
                        type,
                        make_unique<handler_data<Event>>(pending_actions)
                    );
                }

                auto& data = *((handler_data<Event>*) it->second.get());
                data.handlers[p].emplace_back(id, std::move(handler));
            };

            if (currently_dispatched == ctti::unnamed_type_id<Event>()) pending_actions.emplace_back(std::move(action));
            else action();
            
            return next_id++;
        }


        // Deduce event type from function signature
        template <
            typename Fn,
            typename Event = std::remove_cvref_t<typename meta::function_traits<Fn>::arguments::head>
        > handler_id add_handler(Fn&& handler, Priority p = Priority(0)) {
            return add_handler<Event>(fwd(handler), p);
        }


        template <typename Event>
        handler_id add_one_time_handler(handler_t<Event> handler, Priority p = Priority(0)) {
            std::lock_guard lock { mtx };

            auto wrapping_handler = [this, id = next_id, handler = std::move(handler)] (const Event& event) mutable {
                handler(event);
                remove_handler<Event>(id);
            };

            return add_handler<Event>(std::move(wrapping_handler), p);
        }


        // Deduce event type from function signature
        template <
            typename Fn,
            typename Event = std::remove_cvref_t<typename meta::function_traits<Fn>::arguments::head>
        > handler_id add_one_time_handler(Fn&& handler, Priority p = Priority(0)) {
            return add_one_time_handler<Event>(fwd(handler), p);
        }
        
        
        template <typename Event>
        void remove_handler(handler_id id) {
            std::lock_guard lock { mtx };

            auto action = [this, id] {
                if (auto it = handlers.find(ctti::unnamed_type_id<Event>()); it != handlers.end()) {
                    it->second->erase(id);
                }
            };

            if (currently_dispatched == ctti::unnamed_type_id<Event>()) pending_actions.emplace_back(std::move(action));
            else action();
        }
        
        
        template <typename Event>
        void add_event(Event event) {
            std::lock_guard lock { mtx };

            // No currently_dispatched check necessary here:
            // if this event type is currently being handled, this event will simply be handled as well.
            auto type = ctti::unnamed_type_id<Event>();

            auto it = handlers.find(type);
            if (it == handlers.end()) {
                std::tie(it, std::ignore) = handlers.emplace(
                    type,
                    make_unique<handler_data<Event>>(pending_actions)
                );
            }

            // Note: add will move the event, but it cannot be passed as Event&& due to type erasure.
            it->second->add(&event);
            handlers_with_events.insert(type);

            has_events = true;
        }
        
        
        void dispatch_events(void) {
            std::lock_guard lock { mtx };

            // Cannot foreach, since handlers may insert new events.
            while (!handlers_with_events.empty()) {
                // Cannot extract, since not all containers (e.g. boost's flat_set) support this.
                auto type = std::move(*handlers_with_events.begin());
                handlers_with_events.erase(handlers_with_events.begin());

                currently_dispatched = type;
                handlers[type]->invoke();
                currently_dispatched = std::nullopt;
            }

            has_events = false;
        }


        // Does this dispatcher have any handlers for the given event type?
        // This can be used as an optimisation before dispatching a large number of events of the same type.
        // Note: if this method returns false, event dispatching can be safely skipped, but this method returning true does not guarantee there are handlers.
        template <typename Event> bool has_handlers_for(void) {
            std::lock_guard lock { mtx };

            // Cannot check, assume there are handlers.
            if (!pending_actions.empty()) return true;

            if (auto it = handlers.find(ctti::type_id<Event>()); it != handlers.end()) {
                return !it->second.empty();
            }

            return false;
        }


        bool has_pending_actions(void) const {
            std::lock_guard lock { mtx };
            return !pending_actions.empty();
        }


        bool has_pending_events(void) const {
            std::lock_guard lock { mtx };
            return has_events;
        }
    private:
        struct handler_data_base {
            virtual ~handler_data_base(void) = default;
            virtual void add(void* event) = 0;
            virtual void invoke(void) = 0;
            virtual void erase(handler_id id) = 0;
            virtual bool empty(void) const = 0;
        };
        
        template <typename Event> struct handler_data : handler_data_base {
            vec_map<Priority, std::vector<std::pair<handler_id, handler_t<Event>>>> handlers;
            std::vector<Event> events;
            std::vector<std::function<void(void)>>& pending_actions;


            explicit handler_data(std::vector<std::function<void(void)>>& pending_actions)
                : pending_actions(pending_actions) {}

            
            void add(void* event) override {
                events.push_back(std::move(*((Event*) event)));
            }
            
            void invoke(void) override {
                // Note: handlers may dispatch additional events, so we can't use iterators here.
                for (std::size_t i = 0; i < events.size(); ++i) {
                    auto event = std::move(events[i]);

                    for (auto& [p, handlers_for_p] : handlers | views::reverse) {
                        for (auto& [id, handler] : handlers_for_p) {
                            if constexpr (Cancellable) {
                                if (std::invoke(handler, event)) goto event_done;
                            } else {
                                std::invoke(handler, event);
                            }
                        }
                    }

                    event_done:
                    if (!pending_actions.empty()) [[unlikely]] {
                        for (auto& action : pending_actions) action();
                        pending_actions.clear();
                    }
                }

                events.clear();
            }
            
            void erase(handler_id id) override {
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
        hash_set<ctti::type_index> handlers_with_events;
        handler_id next_id = 0;
        mutable lock_t mtx;

        std::optional<ctti::type_index> currently_dispatched;
        std::vector<std::function<void(void)>> pending_actions;
        bool has_events = false;
    };
}