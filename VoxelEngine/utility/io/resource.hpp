#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


#define VE_IMPL_RESOURCE_TYPE(name, path) \
inline static_resource name##_resource(std::string_view filename) { return { path, filename }; }


namespace ve::io {
    // In places sensitive to performance, it makes more sense to pass each resource as a reference to its directory,
    // and a view of its name.
    // In the few cases where the URI is constructed dynamically at runtime, those doing so can just manage
    // the lifetime of these resources themselves.
    struct static_resource {
        const fs::path& resource_directory;
        std::string_view name;
        
        ve_hashable(resource_directory, name);
        ve_eq_comparable_fields(static_resource, resource_directory, name);
        
        explicit operator fs::path(void) const {
            return resource_directory / name;
        }
    };
    
    
    VE_IMPL_RESOURCE_TYPE(tile,   paths::PATH_TILE_TEXTURES);
    VE_IMPL_RESOURCE_TYPE(shader, paths::PATH_SHADERS);
    VE_IMPL_RESOURCE_TYPE(sound,  paths::PATH_SOUNDS);
    VE_IMPL_RESOURCE_TYPE(asset,  paths::PATH_ASSETS);
}