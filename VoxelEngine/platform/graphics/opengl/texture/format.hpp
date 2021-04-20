#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/constexpr_string_ops.hpp>

#include <GL/glew.h>
#include <ctre.hpp>
#include <magic_enum.hpp>


namespace ve::graphics {
    struct texture_format_data {
        enum class primitive { FLOAT, SIGNED, UNSIGNED, NORMALIZED } primitive;
        enum class type { COLOR, DEPTH } type;
        
        std::size_t channels;
        std::size_t bits_per_channel;
        
        // Format in which the data will be stored on the GPU.
        GLenum storage_format;
        
        // Different from storage format, defines number and order of channels.
        GLenum channel_format;
    };
    
    
    namespace detail {
        constexpr inline ctll::fixed_string texfmt_regex = R"RGX((RGBA?|DUAL|GRAY|DEPTH)(\d+)([UIF]?))RGX";
        
        // Disable compiler warning for branches without return statement.
        // Missing return statements are intentional, since this method is consteval,
        // errors should cause compilation failure.
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wreturn-type"
        consteval texture_format_data parse_texture_format_string(std::string_view name, GLenum value) {
            using prims = enum class texture_format_data::primitive;
            using types = enum class texture_format_data::type;
        
            if (auto [match, type, bits, primitive] = ctre::match<texfmt_regex>(name); match) {
                texture_format_data result { };
            
            
                // Primitive
                if      (primitive.size() == 0) result.primitive = prims::NORMALIZED;
                else if (primitive == "F") result.primitive = prims::FLOAT;
                else if (primitive == "I") result.primitive = prims::SIGNED;
                else if (primitive == "U") result.primitive = prims::UNSIGNED;
            
            
                // Texture type
                result.type = (type == "DEPTH")
                              ? types::DEPTH
                              : types::COLOR;
            
            
                // Number of channels & channel size
                result.channels = 1;
                if      (type == "RGBA") result.channels = 4;
                else if (type == "RGB" ) result.channels = 3;
                else if (type == "DUAL") result.channels = 2;
            
                result.bits_per_channel = constexpr_stoi<std::size_t>(bits) / result.channels;
                
            
                // GL constants for storage format & channel format.
                result.storage_format = value;
                
                
                if      (result.channels == 4) result.channel_format = GL_RGBA;
                else if (result.channels == 3) result.channel_format = GL_RGB;
                else if (result.channels == 2) result.channel_format = GL_RG;
                else {
                    result.channel_format == (result.type == types::DEPTH)
                    ? GL_DEPTH_COMPONENT
                    : GL_RED;
                }
            
            
                return result;
            } else {
                // Lack of return statement triggers compilation failure if regex does not match.
            }
        }
        #pragma clang diagnostic pop
    }
    
    
    enum class texture_format : u32 {
        // Color formats: unsigned normalized
        RGBA8, RGB8, DUAL8, GRAY8, RGBA16, RGB16, DUAL16, GRAY16,
        // Color formats: unsigned non-normalized
        RGBA8U, RGB8U, DUAL8U, GRAY8U, RGBA16U, RGB16U, DUAL16U, GRAY16U, RGBA32U, RGB32U, DUAL32U, GRAY32U,
        // Color formats: signed non-normalized
        RGBA8I, RGB8I, DUAL8I, GRAY8I, RGBA16I, RGB16I, DUAL16I, GRAY16I, RGBA32I, RGB32I, DUAL32I, GRAY32I,
        // Color formats: floating point
        RGBA16F, RGB16F, DUAL16F, GRAY16F, RGBA32F, RGB32F, DUAL32F, GRAY32F,
        // Depth formats
        DEPTH16, DEPTH24, DEPTH32, DEPTH32F
    };
    
    
    #define VE_IMPL_TEXFMT(name, value)  detail::parse_texture_format_string(#name, value)
    
    constexpr inline std::array texture_format_info {
        // Color formats: unsigned normalized
        VE_IMPL_TEXFMT(RGBA8,   GL_RGBA8),    VE_IMPL_TEXFMT(RGB8,   GL_RGB8),    VE_IMPL_TEXFMT(DUAL8,   GL_RG8),    VE_IMPL_TEXFMT(GRAY8,   GL_R8),
        VE_IMPL_TEXFMT(RGBA16,  GL_RGBA16),   VE_IMPL_TEXFMT(RGB16,  GL_RGB16),   VE_IMPL_TEXFMT(DUAL16,  GL_RG16),   VE_IMPL_TEXFMT(GRAY16,  GL_R16),
        // Color formats: unsigned non-normalized
        VE_IMPL_TEXFMT(RGBA8U,  GL_RGBA8UI),  VE_IMPL_TEXFMT(RGB8U,  GL_RGB8UI),  VE_IMPL_TEXFMT(DUAL8U,  GL_RG8UI),  VE_IMPL_TEXFMT(GRAY8U,  GL_R8UI),
        VE_IMPL_TEXFMT(RGBA16U, GL_RGBA16UI), VE_IMPL_TEXFMT(RGB16U, GL_RGB16UI), VE_IMPL_TEXFMT(DUAL16U, GL_RG16UI), VE_IMPL_TEXFMT(GRAY16U, GL_R16UI),
        VE_IMPL_TEXFMT(RGBA32U, GL_RGBA32UI), VE_IMPL_TEXFMT(RGB32U, GL_RGB32UI), VE_IMPL_TEXFMT(DUAL32U, GL_RG32UI), VE_IMPL_TEXFMT(GRAY32U, GL_R32UI),
        // Color formats: signed non-normalized
        VE_IMPL_TEXFMT(RGBA8I,  GL_RGBA8I),   VE_IMPL_TEXFMT(RGB8I,  GL_RGB8I),   VE_IMPL_TEXFMT(DUAL8I,  GL_RG8I),   VE_IMPL_TEXFMT(GRAY8I,  GL_R8I),
        VE_IMPL_TEXFMT(RGBA16I, GL_RGBA16I),  VE_IMPL_TEXFMT(RGB16I, GL_RGB16I),  VE_IMPL_TEXFMT(DUAL16I, GL_RG16I),  VE_IMPL_TEXFMT(GRAY16I, GL_R16I),
        VE_IMPL_TEXFMT(RGBA32I, GL_RGBA32I),  VE_IMPL_TEXFMT(RGB32I, GL_RGB32I),  VE_IMPL_TEXFMT(DUAL32I, GL_RG32I),  VE_IMPL_TEXFMT(GRAY32I, GL_R32I),
        // Color formats: floating point
        VE_IMPL_TEXFMT(RGBA16F, GL_RGBA16F),  VE_IMPL_TEXFMT(RGB16F, GL_RGB16F),  VE_IMPL_TEXFMT(DUAL16F, GL_RG16F),  VE_IMPL_TEXFMT(GRAY16F, GL_R16F),
        VE_IMPL_TEXFMT(RGBA32F, GL_RGBA32F),  VE_IMPL_TEXFMT(RGB32F, GL_RGB32F),  VE_IMPL_TEXFMT(DUAL32F, GL_RG32F),  VE_IMPL_TEXFMT(GRAY32F, GL_R32F),
        // Depth formats
        VE_IMPL_TEXFMT(DEPTH16, GL_DEPTH_COMPONENT16),
        VE_IMPL_TEXFMT(DEPTH24, GL_DEPTH_COMPONENT24),
        VE_IMPL_TEXFMT(DEPTH32, GL_DEPTH_COMPONENT32),
        VE_IMPL_TEXFMT(DEPTH32F, GL_DEPTH_COMPONENT32F)
    };
}