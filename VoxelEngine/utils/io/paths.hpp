#pragma once

#include <VoxelEngine/core/core.hpp>

#include <filesystem>
#include <array>


namespace ve::io::paths {
    ve_init_order(0) const inline fs::path ROOT_DIR                      = fs::absolute(fs::current_path());
    ve_init_order(0) const inline fs::path PATH_LOGS                     = ROOT_DIR / "logs/";
    ve_init_order(0) const inline fs::path PATH_SETTINGS                 = ROOT_DIR / "cfg/";
    ve_init_order(0) const inline fs::path PATH_PLUGINS                  = ROOT_DIR / "plugins/";
    ve_init_order(0) const inline fs::path PATH_ASSETS                   = ROOT_DIR / "assets/";
    ve_init_order(0) const inline fs::path PATH_SHADERS                  = PATH_ASSETS / "shaders/";
    ve_init_order(0) const inline fs::path PATH_TEXTURES                 = PATH_ASSETS / "textures/";
    ve_init_order(0) const inline fs::path PATH_SOUNDS                   = PATH_ASSETS / "sounds/";
    
    
    inline const auto& get_required_paths(void) {
        const static std::array paths {
            PATH_LOGS, PATH_SETTINGS, PATH_PLUGINS, PATH_ASSETS,
            PATH_SHADERS, PATH_TEXTURES, PATH_SOUNDS
        };
        
        return paths;
    }
}