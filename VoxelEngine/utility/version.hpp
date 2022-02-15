#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/string.hpp>


namespace ve {
    struct version {
        u32 major, minor, patch;
        std::string details = "";
        
        explicit operator std::string(void) const {
            return details.empty()
                ? cat(major, ".", minor, ".", patch)
                : cat(details, " ", major, ".", minor, ".", patch);
        }
    
        ve_field_comparable(version, major, minor, patch);
    };
    
    
    struct version_range {
        std::optional<version> min_version = std::nullopt, max_version = std::nullopt;
        
        explicit operator std::string(void) const {
            std::string version_range;
    
            if (min_version && !max_version) {
                version_range = cat(*min_version, " or newer");
            } else if (min_version) {
                version_range = cat("between ", *min_version, " and ", *max_version);
            } else {
                version_range = cat(*max_version, " or older");
            }
            
            return version_range;
        }
        
        constexpr bool contains(const version& ver) const {
            if (min_version && ver < *min_version) return false;
            if (max_version && ver > *max_version) return false;
            return true;
        }
    };
}