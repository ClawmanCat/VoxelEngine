#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side.hpp>
#include <VoxelEngine/dependent/actor.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/ecs/scene_registry.hpp>
#include <VoxelEngine/ecs/system/system_view_type.hpp>
#include <VoxelEngine/utility/priority.hpp>

#include <entt/entt.hpp>

#include <string>


namespace ve {
    template <side Side> class entity_base;
    
    
    template <side Side> class scene : public resource_owner<scene<Side>> {
    public:
        using entity_t = entity_base<Side>;
        friend entity_t;
        
        
        explicit scene(ve_default_actor(owner)) : owner(owner) {
            scene_registry<Side>::instance().add_scene(this);
        }
        
        ~scene(void) {
            scene_registry<Side>::instance().remove_scene(this);
        }
        
        scene(scene&& o) {
            scene_registry<Side>::instance().add_scene(this);
            *this = std::move(o);
        }
        
        scene& operator=(scene&& o) {
            std::swap(storage,  o.storage);
            std::swap(entities, o.entities);
            std::swap(systems,  o.systems);
            std::swap(owner,    o.owner);
            
            return *this;
        }
        
        scene(const scene&) = delete;
        scene& operator=(const scene&) = delete;
        
        
        void on_actor_destroyed(actor_id id) {
            if (auto it = entities.find(id); it != entities.end()) {
                it->second.clear();
            }
            
            systems.erase(
                std::remove_if(
                    systems.begin(),
                    systems.end(),
                    [&](const auto& kv) { return kv.second.owner == id; }
                ),
                systems.end()
            );
        }
        
        
        void update(microseconds dt) {
            for (auto& [priority, storage] : systems | views::reverse) storage.system_fn(dt);
        }
        
        
        template <typename T, typename... Args> requires std::is_base_of_v<entity_t, T>
        T& create_entity(ve_default_actor(owner), Args&&... args) {
            auto [it, changed] = entities[owner].emplace(std::make_unique<T>(
                typename entity_t::secret_type { },
                this,
                storage.create(),
                owner
            ));
            
            T& entity = static_cast<T&>(**it);
            entity.init(std::forward<Args>(args)...);
            
            return entity;
        }
        
        
        void destroy_entity(entity_t& e) {
            entities[e.owner].erase(&e);
            storage.destroy(e.id);
    
            e.scene = nullptr;
            e.id    = entt::null;
        }
        
        
        template <typename System> requires requires { typename System::ecs_system_tag; }
        void add_system(System&& system, priority p = priority::NORMAL, ve_default_actor(owner)) {
            systems.emplace(std::pair {
                p,
                system_storage {
                    owner,
                    [this, system = std::move(system)] (microseconds dt) mutable {
                        auto view = detail::generate_view_type<
                            typename System::components,
                            typename System::excluded
                        >::registry_getter(storage);
        
        
                        system.update(view, dt);
                    }
                }
            });
        }
        
        
        VE_GET_VAL(owner);
    private:
        struct system_storage {
            actor_id owner;
            std::function<void(microseconds)> system_fn;
        };
        
        
        entt::registry storage;
        flat_map<actor_id, hash_set<unique<entity_t>>> entities;
        flat_multimap<priority, system_storage> systems;
        actor_id owner;
    };
}