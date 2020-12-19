#include <VoxelEngine/entity/component/required_component_registry.hpp>


namespace ve {
    required_component_registry& required_component_registry::instance(void) {
        static required_component_registry i;
        return i;
    }
    
    
    void required_component_registry::add(ctti::type_id_t cls, ctti::type_id_t component, actor_id owner) {
        components[cls].push_back(component);
        
        auto& owner_storage = owners[owner];
        if (!contains(cls, owner_storage)) owner_storage.push_back(cls);
    }
    
    
    bool required_component_registry::is_required(ctti::type_id_t cls, ctti::type_id_t component) const {
        auto it = components.find(cls);
        if (it == components.end()) return false;
        
        return contains(component, it->second);
    }
    
    
    void required_component_registry::on_actor_destroyed(actor_id id) {
        auto it = owners.find(id);
        
        if (it != owners.end()) {
            for (const auto& cls : it->second) components.erase(cls);
            owners.erase(it);
        }
    }
}