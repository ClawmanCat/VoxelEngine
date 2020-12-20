#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/entity/component/component_info.hpp>

#include <entt/entt.hpp>


namespace ve {
    class entity {
    public:
        entity(void) : owner(nullptr), id(entt::null) {}
        entity(entt::registry* owner) : owner(owner), id(owner->create()) {}
        
        entity(const entity&) = delete;
        entity& operator=(const entity&) = delete;
        
        entity(entity&& o) : entity() {
            *this = std::move(o);
        }
        
        entity& operator=(entity&& o) {
            std::swap(owner, o.owner);
            std::swap(id, o.id);
            
            return *this;
        }
        
        virtual ~entity(void) {
            if (owner && id != entt::null) {
                owner->destroy(id);
            }
        }
        
        
        virtual std::vector<data_component_info> get_static_component_members(void) const {
            const static std::vector<data_component_info> components {
                VE_DATA_COMPONENT(dynamic)
            };
            
            return components;
        }
    
        virtual std::vector<function_component_info> get_static_component_methods(void) const {
            return { };
        }
    
    
        // If an entity is dynamic, one or more of its component methods have been replaced by one from the registry.
        bool is_dynamic(void) const { return dynamic; }
        void set_dynamic(bool dynamic) { this->dynamic = dynamic; }
        
        entt::entity get_id(void) const { return id; }
    protected:
        entt::registry* owner;
        entt::entity id;
        
    private:
        bool dynamic = false;
    };
}