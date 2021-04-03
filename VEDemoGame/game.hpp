#pragma once

#include <VEDemoGame/core/core.hpp>
#include <VoxelEngine/dependent/game.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/voxel/voxel.hpp>
#include <VoxelEngine/ecs/ecs.hpp>


namespace demo_game {
    using namespace ve::namespaces;
    namespace gfx   = ve::graphics;
    namespace paths = ve::io::paths;
    
    
    // Meyers Singleton is used here to prevent initialization order fiasco w.r.t. tiles.
    extern ve::simple_tile_storage& get_tile_store(void);
    
    extern const ve::tile* tile_grass;
    extern const ve::tile* tile_stone;
    
    
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
            voxels.add_loader(std::make_shared<ve::point_loader<>>(
                &voxels,
                ve::vec3i { 0 },
                ve::vec3i { 3 }
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