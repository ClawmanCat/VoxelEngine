#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/math.hpp>

#include <VoxelEngine/platform/graphics/graphics.hpp>
#include graphics_include(buffer/vertex.hpp)
#include graphics_include(buffer/indexed_vertex_buffer.hpp)
#include graphics_include(texture/texture_atlas.hpp)

#include <array>


namespace ve::graphics {
    // Helper for matching cube faces to indices.
    struct cube_textures {
        subtexture bottom, top, back, front, left, right;
        
        operator std::array<subtexture, 6>(void) const {
            return { bottom, top, back, front, left, right };
        }
    };
    
    
    
    // TODO: Owner should be the owner of the subtexture implicitly.
    inline shared<buffer> textured_cube(const std::array<subtexture, 6 * 4>& vertex_textures, ve_default_actor(owner)) {
        using vertex = flat_texture_vertex_3d;
    
        std::array axis_orderings {
            std::array { &vec3f::y, &vec3f::x, &vec3f::z },
            std::array { &vec3f::z, &vec3f::x, &vec3f::y },
            std::array { &vec3f::x, &vec3f::z, &vec3f::y }
        };
        
        std::array<Fn<vec2f, const subtexture&>, 4> uv_orderings {
            [](const subtexture& st) { return st.uv + (st.size * vec2f { 1, 1 }); },
            [](const subtexture& st) { return st.uv + (st.size * vec2f { 1, 0 }); },
            [](const subtexture& st) { return st.uv + (st.size * vec2f { 0, 1 }); },
            [](const subtexture& st) { return st.uv + (st.size * vec2f { 0, 0 }); }
        };
    
        constexpr std::array offsets = { -0.5f, 0.5f };
        
        
        // Each texture used in the cube requires a separate ID so we can assign a texture unit to it later.
        // TODO: Dynamically indexing the uniform array is not supported with OpenGL ES.
        // TODO: Find a better way to do this.
        flat_map<shared<texture>, u8> texture_indices;
        auto get_tex_index = [&](const subtexture& st) {
            if (auto it = texture_indices.find(st.tex); it != texture_indices.end()) {
                return it->second;
            } else {
                texture_indices[st.tex] = texture_indices.size();
                return (u8) (texture_indices.size() - 1);
            }
        };
        
        
        u32 index = 0;
        std::array<vertex, 6 * 4> vertices;
        
        for (const auto& [first, second, third] : axis_orderings) {
            for (auto first_offset : offsets) {
                std::size_t tex_face = 0;
                
                for (auto second_offset : offsets) {
                    for (auto third_offset : offsets) {
                        vec3f position;
                        position.*first  = first_offset;
                        position.*second = second_offset;
                        position.*third  = third_offset;
                    
                        std::size_t actual_face = tex_face++;
                        // Front side (3) and left side (4) should be flipped.
                        if (in(index, 3u * 4, 4u * 4) || in(index, 4u * 4, 5u * 4)) actual_face = (actual_face + 2) % 4;
                        
                        vertices[index] = vertex {
                            .position  = position,
                            .uv        = uv_orderings[actual_face](vertex_textures[index]),
                            .tex_index = get_tex_index(vertex_textures[index])
                        };
                        
                        ++index;
                    }
                }
            }
        }
    
        constexpr std::array index_pattern = { 0, 2, 3, 0, 3, 1 };
        std::array<u32, 6 * 6> indices;
        index = 0;
    
        for (std::size_t side = 0; side < 6; ++side) {
            for (auto vert_index : index_pattern) {
                indices[index++] = (4 * side) + vert_index;
            }
        }
    
    
        auto buffer = std::make_shared<indexed_vertex_buffer<vertex>>(
            std::span { vertices },
            std::span { indices  }
        );
        
        for (auto& [tex, id] : texture_indices) {
            buffer->set_uniform_value("tex["s + std::to_string(id) + "]", std::move(tex), owner);
        }
        
        return buffer;
    }
    
    
    inline shared<buffer> textured_cube(const subtexture& cube_texture, ve_default_actor(owner)) {
        std::array<subtexture, 6 * 4> vertex_textures;
        vertex_textures.fill(cube_texture);
        
        return textured_cube(vertex_textures, owner);
    }
    
    
    inline shared<buffer> textured_cube(const std::array<subtexture, 6>& side_textures, ve_default_actor(owner)) {
        std::array<subtexture, 6 * 4> vertex_textures;
    
        for (std::size_t side = 0; side < 6; ++side) {
            std::fill(
                vertex_textures.begin() + (4 * side),
                vertex_textures.begin() + (4 * side) + 4,
                side_textures[side]
            );
        }
    
        return textured_cube(vertex_textures, owner);
    }
}