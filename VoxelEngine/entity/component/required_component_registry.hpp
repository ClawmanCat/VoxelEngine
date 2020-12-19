#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/dependent/resource_owner.hpp>
#include <VoxelEngine/utils/container_utils.hpp>

#include <ctti/type_id.hpp>

#include <vector>


namespace ve {
    class required_component_registry : public resource_owner<required_component_registry> {
    public:
        static required_component_registry& instance(void);
        
        void VE_RESOURCE_FN(add, ctti::type_id_t cls, ctti::type_id_t component);
        bool is_required(ctti::type_id_t cls, ctti::type_id_t component) const;
    
        void on_actor_destroyed(actor_id id);
    private:
        required_component_registry(void) = default;
        
        hash_map<ctti::type_id_t, std::vector<ctti::type_id_t>> components;
        hash_map<actor_id, std::vector<ctti::type_id_t>> owners;
    };
}