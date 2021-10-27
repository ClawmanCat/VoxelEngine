#include <VoxelEngine/clientserver/instance.hpp>


namespace ve {
    instance_registry& instance_registry::instance(void) {
        static instance_registry i { };
        return i;
    }


    void instance_registry::update_all(nanoseconds dt) {
        for (auto* instance : instances) instance->update(dt);
    }
}