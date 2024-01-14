#include <VoxelEngine/core/ioc.hpp>


namespace ve {
    [[nodiscard]] kgr::container& services(void) {
        static kgr::container instance { };
        return instance;
    }
}