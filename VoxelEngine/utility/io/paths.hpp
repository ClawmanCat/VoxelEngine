#pragma once

#include <VoxelEngine/core/core.hpp>

#include <filesystem>
#include <array>


namespace ve::io::paths {
    const inline fs::path ROOT_DIR                          = fs::absolute(fs::current_path());
    const inline fs::path PATH_LOGS                         = ROOT_DIR / "logs/";
    const inline fs::path PATH_SETTINGS                     = ROOT_DIR / "cfg/";
    const inline fs::path PATH_PLUGINS                      = ROOT_DIR / "plugins/";
    const inline fs::path PATH_ENGINE_DATA                  = ROOT_DIR / "data/";
    const inline fs::path PATH_ASSETS                       = ROOT_DIR / "assets/";
    const inline fs::path PATH_SHADERS                      = PATH_ASSETS / "shaders/";
    const inline fs::path PATH_TEXTURES                     = PATH_ASSETS / "textures/";
    const inline fs::path PATH_SOUNDS                       = PATH_ASSETS / "sounds/";
    const inline fs::path PATH_TILE_TEXTURES                = PATH_TEXTURES / "tiles/";
    const inline fs::path PATH_ENTITY_TEXTURES              = PATH_TEXTURES / "entities/";
    
    
    inline const auto& get_required_paths(void) {
        const static std::array paths {
            PATH_LOGS, PATH_SETTINGS, PATH_PLUGINS, PATH_ENGINE_DATA, PATH_ASSETS,
            PATH_SHADERS, PATH_TEXTURES, PATH_SOUNDS, PATH_TILE_TEXTURES
        };
        
        return paths;
    }
}