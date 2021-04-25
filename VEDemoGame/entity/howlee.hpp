#pragma once

#include <VEDemoGame/core/core.hpp>
#include <VEDemoGame/game.hpp>
#include <VEDemoGame/tile/tiles.hpp>

#include <VoxelEngine/voxel/voxel.hpp>
#include <VoxelEngine/voxel/tile/tile.hpp>
#include <VoxelEngine/utility/distance.hpp>
#include <VoxelEngine/utility/random.hpp>


namespace demo_game {
    class howlee : public ve::entity<howlee, ve::side::CLIENT> {
    public:
        enum type { NORMAL, ASIMOV, BAD };
        
        
        using base = ve::entity<howlee, ve::side::CLIENT>;
        using base::base;
        
        
        void init(void) {
            using vertex = gfx::flat_texture_vertex_3d;
            
            ve::u32 id = ve::u32(get_id());
            if ((id + 0) % 50  == 0) howlee_type = ASIMOV;
            if ((id + 1) % 100 == 0) howlee_type = BAD;
            
            
            gfx::subtexture texture = *ve::voxel_settings::tile_texture_manager.get_texture(
                ve::io::paths::PATH_ENTITY_TEXTURES /
                (howlee_type == NORMAL ? "howlee.png"s :
                    howlee_type == ASIMOV ? "asimov.png"s : "bad_howlee.png"s)
            );
            
            
            auto uv = texture.uv;
            auto sz = texture.size;
            
            std::vector<vertex> vertices {
                vertex { .position = { -0.5, 0, 0 }, .uv = uv + ve::vec2f { 0, 1 } * sz, .tex_index = texture.index  },
                vertex { .position = { -0.5, 1, 0 }, .uv = uv + ve::vec2f { 0, 0 } * sz, .tex_index = texture.index  },
                vertex { .position = { +0.5, 0, 0 }, .uv = uv + ve::vec2f { 1, 1 } * sz, .tex_index = texture.index  },
                vertex { .position = { +0.5, 1, 0 }, .uv = uv + ve::vec2f { 1, 0 } * sz, .tex_index = texture.index  }
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

            const float dt_seconds    = (float(dt.count()) / 1e6f);
            const float velocity      = 3.5f;
            const float random        = cheaprand::random_real();
            const float change_chance = 0.25f * dt_seconds;
            const bool  change_dir    = random < change_chance;
            
            
            // Randomly change direction.
            if (!falling && change_dir) {
                const float random_dir = random * (1.0f / change_chance);
                
                direction dir;
                if      (random_dir < 0.25f) dir = direction::NORTH;
                else if (random_dir < 0.50f) dir = direction::SOUTH;
                else if (random_dir < 0.75f) dir = direction::EAST;
                else                         dir = direction::WEST;
                
                transform.linear_velocity = velocity * (vec3f) direction_vector(dir);
            }
            
            
            // Randomly jump.
            if (!falling) {
                const float random_jump = cheaprand::random_real();
                const float jump_chance = 0.002f * dt_seconds;
                const bool  jump        = random_jump < jump_chance;
                
                if (jump) {
                    transform.linear_velocity.y += cheaprand::random_real(5.0f, 20.0f);
                    
                    transform.linear_velocity.x += cheaprand::random_real(-10.0f, 10.0f);
                    transform.linear_velocity.z += cheaprand::random_real(-10.0f, 10.0f);
                    return;
                }
            }
            
            
            // Stop if moving would cause collision.
            for (const auto& axis : std::array { &vec3f::x, &vec3f::z }) {
                vec3f velocity_axis { 0 };
                velocity_axis.*axis = transform.linear_velocity.*axis;
                
                auto target = game::world->voxels.get_tile(
                    ve::tilepos { transform.position + velocity_axis }
                );
    
                bool axis_free = target == ve::tiles::TILE_VOID || target == ve::tiles::TILE_UNKNOWN;
                transform.linear_velocity.*axis *= axis_free;
            }

            
            // Fall if there is no tile below us.
            falling = true;
            
            for (float dx : std::array { -0.5f, +0.5f }) {
                falling &= one_of(
                    game::world->voxels.get_tile(
                        ve::tilepos { transform.position + vec3f { dx, -1, 0 } }
                    ),
                    ve::tiles::TILE_VOID,
                    ve::tiles::TILE_UNKNOWN
                );
            }
            
            if (falling) {
                if (transform.linear_velocity.y > -9.81f) {
                    transform.linear_velocity.y -= 9.81f * dt_seconds;
                    transform.linear_velocity.x *= std::pow(0.5f, dt_seconds);
                    transform.linear_velocity.z *= std::pow(0.5f, dt_seconds);
                }
            } else {
                transform.linear_velocity.y = 0;
            }
            
            
            // Special Howlees can destroy tiles.
            if (howlee_type == ASIMOV && !falling) {
                const float destroy_random = cheaprand::random_real();
                const float destroy_chance = 0.01f * dt_seconds;
                const bool  destroy        = destroy_random < destroy_chance;
                
                if (destroy) {
                    distance_functions::euclidean<tilepos> fn;
                    auto center = tilepos { transform.position - vec3f { 0.f, 0.5f, 0.f } };
                    
                    spatial_iterate(
                        center,
                        tilepos { 5 },
                        [&](const auto& pos) {
                            if (fn.within(pos, center, 5)) {
                                game::world->voxels.set_tile(pos, tiles::TILE_VOID);
                            }
                        }
                    );
                }
            }
            
            
            // Bad howlees can build
            if (howlee_type == BAD && !falling) {
                const float build_random = cheaprand::random_real();
                const float build_chance = 0.5f * dt_seconds;
                const bool  build        = build_random < build_chance;
                
                if (build) {
                    game::world->voxels.set_tile((tilepos) transform.position, tile_stone);
                    transform.position.y += 1;
                    transform.linear_velocity = vec3f { 0 };
                }
            }
        }


        ve::renderable_component VE_COMPONENT(mesh);
        ve::transform_component VE_COMPONENT(transform);
        bool VE_COMPONENT(falling, SERVER, NONE) = false;
        type VE_COMPONENT(howlee_type, SERVER, NONE) = NORMAL;
    };
}