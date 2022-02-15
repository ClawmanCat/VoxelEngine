#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/version.hpp>


namespace ve {
    struct game_info {
        std::string display_name;
        std::vector<std::string> description;
        std::vector<std::string> authors;
        version version;
    };
    
    
    struct plugin_info {
        std::string display_name, internal_name;
        std::vector<std::string> description;
        std::vector<std::string> authors;
        version version;
        
        
        // Can this plugin be loaded and unloaded during runtime, or only at program startup / exit?
        bool allow_dynamic_load   = true;
        bool allow_dynamic_unload = true;
        
        
        struct dependency {
            std::string internal_name;
            version_range required_version;
            bool required = true;
        };
        
        std::vector<dependency> dependencies;
    };
}