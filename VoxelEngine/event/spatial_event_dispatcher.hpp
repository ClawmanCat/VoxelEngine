#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/distance.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/utility.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>

#include <ctti/type_id.hpp>


namespace ve {
    // TODO: Implement prioritized event handling.
    template <
        template <typename Pos> typename DistanceFn = distance_functions::L1,
        std::size_t Dimensions    = 3,
        std::size_t Resolution    = 4,
        std::size_t ScaleFactor   = 4,
        std::size_t NumLayers     = 10,
        typename WorldDistance    = float,
        typename LayerDistance    = i32
    > requires (
        std::is_integral_v<LayerDistance> &&
        power_of_2(Resolution)            &&
        power_of_2(ScaleFactor)
    ) class spatial_event_dispatcher :
        public resource_owner<spatial_event_dispatcher<
            DistanceFn,
            Dimensions,
            Resolution,
            ScaleFactor,
            NumLayers,
            WorldDistance,
            LayerDistance
        >>
    {
    public:
        using spatial_event_dispatcher_tag = void;
        
        using WorldPosition = vec<Dimensions, WorldDistance>;
        using LayerPosition = vec<Dimensions, LayerDistance>;
        
        constexpr static auto WorldDistanceFn = DistanceFn<WorldPosition> { };
        
        
        void on_actor_destroyed(actor_id id) {
            auto it = handler_owners.find(id);
            if (it == handler_owners.end()) return;
            
            for (const auto& owner_data : it->second) {
                std::vector<handler_data>* ctr;
                
                if (owner_data.is_global) {
                    ctr = &global_handlers[owner_data.event_type];
                } else {
                    if (owner_data.layer < NumLayers) {
                        auto& layer = layers[owner_data.layer];
                        ctr = &layer.handlers[to_layerpos(owner_data.position, layer.resolution)];
                    } else {
                        ctr = &oversized_handlers[owner_data.event_type];
                    }
                }
                
                swap_erase(
                    *ctr,
                    std::find_if(ctr->begin(), ctr->end(), ve_field_equals(id, owner_data.id))
                );
            }
            
            handler_owners.erase(it);
        }
        
        
        // Adds a handler that fires if an event of the given type occurs within the given distance of the given position.
        template <typename Event> requires std::is_base_of_v<event, Event>
        event_handler_id add_handler(event_handler&& handler, const WorldPosition& position, WorldDistance distance, ve_default_actor(owner)) {
            auto type  = ctti::type_id<Event>();
            auto id    = handler.get_id();
            auto layer = layer_for_distance(distance);
            
            auto& ctr = (layer < NumLayers)
                ? layers[layer].handlers[to_layerpos(position, layers[layer].resolution)]
                : oversized_handlers[type];
            
            ctr.push_back(handler_data {
                .handler    = std::forward<event_handler>(handler),
                .position   = position,
                .distance   = distance,
                .event_type = type,
                .id         = id,
                .owner      = owner
            });
            
            handler_owners[owner].insert(handler_lookup_data {
                .position   = position,
                .layer      = layer,
                .event_type = type,
                .id         = id,
                .is_global  = false
            });
            
            return id;
        }
    
    
        template <typename Event> requires std::is_base_of_v<event, Event>
        event_handler_id add_handler(const event_handler& handler, const WorldPosition& position, WorldDistance distance, ve_default_actor(owner)) {
            return add_handler(copy(handler), position, distance, owner);
        }
        
        
        // Adds a handler that fires for all events of the given type, no matter their position.
        template <typename Event> requires std::is_base_of_v<event, Event>
        event_handler_id add_handler(event_handler&& handler, ve_default_actor(owner)) {
            auto type = ctti::type_id<Event>();
            auto id   = handler.get_id();
    
            global_handlers[type].push_back(handler_data {
                .handler    = std::forward<event_handler>(handler),
                .event_type = type,
                .id         = id,
                .owner      = owner
            });
    
            handler_owners[owner].insert(handler_lookup_data {
                .event_type = type,
                .id         = id,
                .is_global  = true
            });
    
            return id;
        }
    
    
        template <typename Event> requires std::is_base_of_v<event, Event>
        event_handler_id add_handler(const event_handler& handler, ve_default_actor(owner)) {
            return add_handler(copy(handler), owner);
        }
    
    
        template <typename Event> requires std::is_base_of_v<event, Event>
        void remove_handler(event_handler_id id, ve_default_actor(owner)) {
            auto lookup_data = std::move(handler_owners[owner].extract(handler_lookup_data { .id = id }).value());
            auto type        = ctti::type_id<Event>();
            
            std::vector<handler_data>* ctr;
            
            if (lookup_data.is_global) {
                ctr = &global_handlers[type];
            } else if (lookup_data.layer >= NumLayers) {
                ctr = &oversized_handlers[type];
            } else {
                auto& layer       = layers[lookup_data.layer];
                auto partition_it = layer.handlers.find(to_layerpos(lookup_data.position, layer.resolution));
                ctr               = &(partition_it->second);
                
                
                // Prevent large amounts of empty partitions when adding and removing large amounts of handlers:
                // just remove the entire partition if it becomes empty.
                if (ctr->size() == 1) {
                    layer.handlers.erase(partition_it);
                    return;
                }
            }
            
            swap_erase(
                *ctr,
                std::find_if(ctr->begin(), ctr->end(), [&](const auto& handler) { return handler.id == id; })
            );
        }
        
        
        // Dispatches an event to all handlers within their specified radius of the event position.
        template <typename Event> requires std::is_base_of_v<event, Event>
        void dispatch_event(const Event& evnt, const WorldPosition& position) {
            auto type = ctti::type_id<Event>();
            
            for (auto& layer : layers) {
                spatial_iterate(to_layerpos(position, layer.resolution), LayerPosition { 1 }, [&](const auto& pos) {
                    for (const auto& handler : layer.handlers[pos]) {
                        if (handler.event_type != type) continue;
                        if (!WorldDistanceFn.within(position, handler.position, handler.distance)) continue;
                        
                        handler.handler(evnt);
                    }
                });
            }
            
            for (const auto& handler : oversized_handlers[type]) {
                if (WorldDistanceFn.within(position, handler.position, handler.distance)) handler.handler(evnt);
            }
            
            for (const auto& handler : global_handlers[type]) handler.handler(evnt);
        }
    
    
        // Dispatches an event globally. All handlers will be called, no matter their position and radius.
        template <typename Event> requires std::is_base_of_v<event, Event>
        void dispatch_event(const Event& evnt) {
            auto type = ctti::type_id<Event>();
            
            for (const auto& layer : layers) {
                for (const auto& [position, handlers] : layer.handlers) {
                    for (const auto& handler : handlers) {
                        if (handler.event_type == type) handler.handler(evnt);
                    }
                }
            }
            
            for (const auto& handler : oversized_handlers[type]) handler.handler(evnt);
            for (const auto& handler : global_handlers[type]) handler.handler(evnt);
        }
        
    private:
        constexpr std::size_t layer_for_distance(WorldDistance distance) {
            // d = R_0 * S^N   =>   N = log(d / R_0) / log(S)
            return (std::size_t) std::ceil(
                std::log(distance / Resolution) /
                std::log((WorldDistance) ScaleFactor)
            );
        }
        
        
        constexpr LayerPosition to_layerpos(const WorldPosition& position, std::size_t resolution) {
            LayerDistance exponent = least_significant_bit(resolution);
            return LayerPosition(position) >> exponent;
        }
        
        
        struct handler_data {
            event_handler handler;
            WorldPosition position;
            WorldDistance distance;
            ctti::type_id_t event_type;
            event_handler_id id;
            actor_id owner;
        };
        
        
        struct handler_lookup_data {
            WorldPosition position;
            std::size_t layer;
            ctti::type_id_t event_type = ctti::type_id<void>();
            event_handler_id id;
            bool is_global;
            
            ve_hashable(id);
            ve_comparable_fields(handler_lookup_data, id);
        };
        
        
        struct partition_layer {
            hash_map<LayerPosition, std::vector<handler_data>> handlers;
            LayerDistance resolution;
            std::size_t removed_since_rehash = 0;
        };
    
    
        std::array<partition_layer, NumLayers> layers = filled_array<partition_layer, NumLayers>([](std::size_t i) {
            return partition_layer { .handlers = {}, .resolution = (LayerDistance) (Resolution * pow(ScaleFactor, i)) };
        });
        
        hash_map<ctti::type_id_t, std::vector<handler_data>> oversized_handlers, global_handlers;
        hash_map<actor_id, hash_set<handler_lookup_data>> handler_owners;
    };
}