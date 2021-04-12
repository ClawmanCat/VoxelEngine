#pragma once

#include <VEDemoGame/core/core.hpp>


namespace demo_game {
    class howlee : public ve::entity<howlee, ve::side::CLIENT> {
    public:
        using base = ve::entity<howlee, ve::side::CLIENT>;
        using base::base;
        
        
        void init(void) {
            using vertex = gfx::flat_texture_vertex_3d;
            
            auto texture = ve::voxel_settings::tile_texture_manager.get_texture(ve::io::paths::PATH_ENTITY_TEXTURES / "howlee.png");
            auto uv = texture->uv;
            auto sz = texture->size;
            
            std::vector<vertex> vertices {
                vertex { .position = { 0, 0, 0 }, .uv = uv + ve::vec2f { 0, 1 } * sz, .tex_index = texture->index  },
                vertex { .position = { 0, 1, 0 }, .uv = uv + ve::vec2f { 0, 0 } * sz, .tex_index = texture->index  },
                vertex { .position = { 1, 0, 0 }, .uv = uv + ve::vec2f { 1, 1 } * sz, .tex_index = texture->index  },
                vertex { .position = { 1, 1, 0 }, .uv = uv + ve::vec2f { 1, 0 } * sz, .tex_index = texture->index  }
            };
            
            std::vector<ve::u32> indices { 0, 2, 3, 0, 3, 1 };
            
            auto buffer = std::make_shared<gfx::indexed_vertex_buffer<vertex, ve::u32>>(
                vertices, indices
            );
            
            mesh.buffers.push_back(gfx::shader_buffer_pair {
                gfx::shader_library::get_shader("atlas_texture_3d"s),
                std::move(buffer)
            });
        }
        
        
        void VE_FUNCTION_COMPONENT(update, SERVER, ve::microseconds dt) {
            using namespace ve;
            
            const float chance = 0.25f * (float(dt.count()) / 1e6f);
            const float random = float(std::rand()) / float(RAND_MAX);
            const float speed  = 3.5f;
            
            if (random < chance) {
                direction direction;
                
                if (random < 0.25f * chance)      direction = direction::NORTH;
                else if (random < 0.50f * chance) direction = direction::SOUTH;
                else if (random < 0.75f * chance) direction = direction::EAST;
                else                              direction = direction::WEST;
                
                transform.linear_velocity = ((vec3f) direction_vector(direction)) * speed;
            }
            
            if (glm::any(glm::abs(transform.position.xz + transform.linear_velocity.xz) > 64.0f)) {
                transform.linear_velocity *= -1.0f;
            }
        }


        VE_PROPER_COMPONENT(mesh, ve::renderable_component);
        VE_PROPER_COMPONENT(transform, ve::transform_component);
    };
}