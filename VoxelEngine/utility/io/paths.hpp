#pragma once

#include <VoxelEngine/core/core.hpp>

#include <filesystem>
#include <array>


namespace ve::io::paths {
    namespace detail {
        inline std::vector<fs::path>& get_registered_paths(void) {
            static std::vector<fs::path> paths;
            return paths;
        }
        
        struct registering_path : fs::path {
            template <typename... Args>
            registering_path(Args&&... args) : fs::path(fwd(args)...) {
                get_registered_paths().push_back(*this);
            }
        };
    }
    
    
    using path = detail::registering_path;
    
    const inline path ROOT_DIR                              = fs::absolute(fs::current_path());
    const inline path PATH_LOGS                             = ROOT_DIR / "logs/";
    const inline path PATH_SETTINGS                         = ROOT_DIR / "cfg/";
    const inline path PATH_PLUGINS                          = ROOT_DIR / "plugins/";
    const inline path PATH_ENGINE_DATA                      = ROOT_DIR / "data/";
    const inline path PATH_ASSETS                           = ROOT_DIR / "assets/";
    // const inline path PATH_SHADERS                          = PATH_ASSETS / "shaders/";
    const inline path PATH_SHADERS                          = ROOT_DIR / "../../../out_dirs/assets/shaders/";
    const inline path PATH_TEXTURES                         = PATH_ASSETS / "textures/";
    const inline path PATH_SOUNDS                           = PATH_ASSETS / "sounds/";
    const inline path PATH_TILE_TEXTURES                    = PATH_TEXTURES / "tiles/";
    const inline path PATH_ENTITY_TEXTURES                  = PATH_TEXTURES / "entities/";
    
    
    inline const std::vector<fs::path>& get_registered_paths(void) {
        return detail::get_registered_paths();
    }
}