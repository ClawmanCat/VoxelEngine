#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/string.hpp>

#include <magic_enum.hpp>


namespace ve::gfx {
    enum class texture_type : u32 {
        COLOR_TEXTURE,
        NORMAL_TEXTURE,
        ROUGHNESS_TEXTURE,
        METALNESS_TEXTURE,
        AMBIENT_OCCLUSION_TEXTURE,
        EMISSIVE_TEXTURE
    };


    constexpr inline std::string_view texture_type_name(texture_type type) {
        switch (type) {
            case texture_type::COLOR_TEXTURE:             return "color";
            case texture_type::NORMAL_TEXTURE:            return "normal";
            case texture_type::ROUGHNESS_TEXTURE:         return "roughness";
            case texture_type::METALNESS_TEXTURE:         return "metalness";
            case texture_type::AMBIENT_OCCLUSION_TEXTURE: return "ao";
            case texture_type::EMISSIVE_TEXTURE:          return "emissive";
        }
    }
}