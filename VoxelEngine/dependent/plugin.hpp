#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/version.hpp>

#include <string>
#include <vector>


namespace ve {
    struct plugin_info {
        // General information
        std::string name;
        std::string internal_name;
        std::vector<std::string> description;
        std::vector<std::string> authors;
        version plugin_version;
        
        // Plugin settings
        bool allow_dynamic_load   = true;
        bool allow_dynamic_unload = true;
    };
}