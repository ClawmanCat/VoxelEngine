#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/on_program_start.hpp>

#include <filesystem>


namespace ve::io::paths {
    namespace fs = std::filesystem;


    const inline fs::path ROOT_DIR      = fs::absolute(fs::current_path());
    const inline fs::path PATH_LOGS     = ROOT_DIR / "logs";
    const inline fs::path PATH_SETTINGS = ROOT_DIR / "cfg";
    const inline fs::path PATH_PLUGINS  = ROOT_DIR / "plugins";
    const inline fs::path PATH_ASSETS   = ROOT_DIR / "assets";
    const inline fs::path PATH_SHADERS  = PATH_ASSETS / "shaders";
    const inline fs::path PATH_TEXTURES = PATH_ASSETS / "textures";
    const inline fs::path PATH_SOUNDS   = PATH_ASSETS / "sounds";
    const inline fs::path PATH_PROFILER = PATH_LOGS / "profiler";


    VE_ON_PROGRAM_START(assure_paths_exist, [] {
        const std::array required_paths {
            PATH_LOGS, PATH_SETTINGS, PATH_PLUGINS, PATH_ASSETS, PATH_SHADERS, PATH_TEXTURES, PATH_SOUNDS
            VE_DEBUG_ONLY(,)
            VE_DEBUG_ONLY(PATH_PROFILER)
        };

        for (const auto& path : required_paths) fs::create_directories(path);
    });
}