#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/immovable.hpp>


namespace ve {
    using actor_destroyed_fn = Fn<void, void*, actor_id>;
    
    
    // The resource owner registry stores a list of classes that own some resource which must be destroyed when
    // the actor associated with that resource is destroyed.
    class resource_owner_registry : public hash_map<void*, actor_destroyed_fn> {
    public:
        static resource_owner_registry& instance(void) {
            static resource_owner_registry i;
            return i;
        }
        
        
        void on_actor_destroyed(actor_id id) {
            for (auto& [owner, fn] : *this) fn(owner, id);
        }
    private:
        resource_owner_registry(void) = default;
        ve_make_immovable;
    };
}