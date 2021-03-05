#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/shader/shader_layout.hpp>
#include <VoxelEngine/utility/io/io.hpp>

#include <ctre.hpp>

#include <tuple>
#include <array>
#include <string>


namespace ve::graphics {
    namespace detail {
        struct glsl_type { GLenum primitive; std::size_t count; };
        
        inline glsl_type parse_glsl_type(std::string_view str) {
            glsl_type result;
            
            // Last character indicates element count (vec3, mat4, etc.) if it exists.
            // Edge case is matAxB for non-square matrices, which we check later.
            constexpr std::string_view digits = "1234";
            if (std::size_t where = digits.find(str.back()); where != std::string_view::npos) {
                result.count = digits[where] - '0';
            } else {
                result.count = 1;
            }
    
    
            // Check for the matAxB case.
            if (auto [match, cols, rows] = ctre::match<".?mat(\\d)x(\\d)">(str); match) {
                result.count *= (cols - '0');
            }
            
            
            // The first character of the type indicates the underlying type.
            // e.g. u = uint, uvecN or umatN.
            // v for vec and m for mat both indicate float as the underlying type.
            switch (str[0]) {
                case 'b': result.primitive = GL_BOOL;         break;
                case 'i': result.primitive = GL_INT;          break;
                case 'u': result.primitive = GL_UNSIGNED_INT; break;
                case 'd': result.primitive = GL_DOUBLE;       break;
                case 'v': [[fallthrough]];
                case 'm': [[fallthrough]];
                case 'f': result.primitive = GL_FLOAT;        break;
                default: VE_ASSERT(false, "Unknown GLSL type: "s + str);
            }
            
            
            return result;
        }
    }
    
    
    // TODO: Do this in a more robust way.
    inline shader_layout get_layout(const io::text_file& vs, const io::text_file& fs) {
        constexpr ctll::fixed_string input_regex  = R"RGX((layout\s*\(.+\))?\s*in\s+(\S+)\s+(\S+)\s*;)RGX";
        constexpr ctll::fixed_string output_regex = R"RGX((layout\s*\(.+\))?\s*out\s+(\S+)\s+(\S+)\s*;)RGX";
        
        shader_layout result;
        
        std::array per_shader_data {
            std::forward_as_tuple(0, vs, result.inputs ),
            std::forward_as_tuple(1, fs, result.outputs)
        };
        
        // TODO: Replace with template-for and integrate regex once it is supported.
        for (auto& [idx, source, target] : per_shader_data) {
            for (const auto& line : source) {
                auto [match, layout, type, name] = (idx == 0)
                    ? ctre::match<input_regex>(line)
                    : ctre::match<output_regex>(line);
    
                if (match) {
                    auto gl_type = detail::parse_glsl_type(type);
            
                    target.push_back(shader_layout::shader_layout_attribute {
                        .name  = std::string { name },
                        .type  = gl_type.primitive,
                        .count = gl_type.count
                    });
                }
            }
        }
        
        return result;
    }
}