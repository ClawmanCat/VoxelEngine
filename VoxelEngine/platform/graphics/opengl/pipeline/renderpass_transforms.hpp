#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::gfx::opengl {
    namespace name_transforms {
        // Transform the name by adding a prefix to it.
        struct add_prefix {
            std::string prefix;

            std::string operator()(std::string_view sv) const {
                return cat(prefix, sv);
            }
        };


        // Replace the given name with a new one and leave all other names unchanged.
        struct rename_one {
            std::string old_name;
            std::string new_name;

            std::string operator()(std::string_view sv) const {
                return sv == old_name ? new_name : std::string { sv };
            }
        };


        // Leave all names unchanged.
        struct identity {
            std::string operator()(std::string_view sv) const {
                return std::string { sv };
            }
        };


        // Map each name in mapping to its value, leave all other names unchanged.
        struct map {
            hash_map<std::string, std::string> mapping;

            std::string operator()(std::string_view sv) const {
                if (auto it = mapping.find(sv); it != mapping.end()) return it->second;
                else return std::string { sv };
            }
        };
    }


    namespace size_transforms {
        struct rescale {
            float factor;

            vec2ui operator()(vec2ui size) const {
                return vec2ui { vec2f { size } * factor };
            }
        };


        struct identity {
            vec2ui operator()(vec2ui size) const {
                return size;
            }
        };
    }
}