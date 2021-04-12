#pragma once

#include <VEDemoGame/core/core.hpp>
#include <VoxelEngine/dependent/game.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/voxel/voxel.hpp>
#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/utility/math.hpp>


namespace demo_game {
    using namespace ve::namespaces;
    namespace gfx   = ve::graphics;
    namespace paths = ve::io::paths;
    
    
    // Meyers Singleton is used here to prevent initialization order fiasco w.r.t. tiles.
    extern ve::simple_tile_storage& get_tile_store(void);
    
    extern const ve::tile* tile_grass;
    extern const ve::tile* tile_stone;
    
    
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
    
        VE_PROPER_COMPONENT(mesh, ve::renderable_component);
        VE_PROPER_COMPONENT(transform, ve::transform_component);
        
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
    };
    
    
    class world : public ve::entity<world, ve::side::CLIENT> {
    public:
        using base = ve::entity<world, ve::side::CLIENT>;
        using base::base;
        
        // Note: components arent valid until the entity is added to a scene,
        // so constructors cannot be used for this.
        void init(void) {
            // Add the voxel space's mesh to be part of this entity's mesh.
            mesh.buffers.push_back(gfx::shader_buffer_pair {
                gfx::shader_library::get_shader("atlas_texture_3d"s),
                voxels.get_mesh()
            });
            
            // Load some chunks in the voxel space.
            voxels.add_loader(std::make_shared<ve::point_loader<ve::distance_functions::L1<ve::chunkpos>{ }>>(
                &voxels, ve::vec3i { 0 }, 3
            ));
        }
        
        
        VE_PROPER_COMPONENT(
            voxels,
            ve::voxel_space,
            std::make_unique<ve::flatland_generator>(
                ve::terrain_layers({
                    { -1, tile_stone },
                    {  0, tile_grass }
                }),
                *ve::tiles::TILE_VOID
            )
        );
        
        
        VE_PROPER_COMPONENT(mesh, ve::renderable_component);
        VE_PROPER_COMPONENT(transform, ve::transform_component);
    };
    
    
    class game {
    public:
        static void on_pre_init(void);
        static void on_post_init(void);
        static void on_pre_loop (ve::u64 tick, microseconds dt);
        static void on_post_loop(ve::u64 tick, microseconds dt);
        static void on_pre_exit(void);
        static void on_post_exit(void);
        
        [[nodiscard]] static const ve::game_info* get_info(void);
        [[nodiscard]] static gfx::window& get_window(void) { return *window; }
        
    private:
        static inline ve::shared<gfx::window> window = nullptr;
        static inline ve::scene<ve::side::CLIENT> scene { };
        static inline gfx::perspective_camera camera { };
    };
}


// Callbacks for the game engine.
namespace ve::game_callbacks {
    void on_game_pre_init(void)  { demo_game::game::on_pre_init();  }
    void on_game_post_init(void) { demo_game::game::on_post_init(); }
    void on_game_pre_loop (ve::u64 tick, microseconds dt) { demo_game::game::on_pre_loop (tick, dt); }
    void on_game_post_loop(ve::u64 tick, microseconds dt) { demo_game::game::on_post_loop(tick, dt); }
    void on_game_pre_exit(void)  { demo_game::game::on_pre_exit();  }
    void on_game_post_exit(void) { demo_game::game::on_post_exit(); }
    
    const ve::game_info* get_game_info(void) {
        return demo_game::game::get_info();
    }
}