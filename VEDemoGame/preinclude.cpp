#include <VEDemoGame/preinclude.hpp>
#include <VEDemoGame/game.hpp>

#include <VoxelEngine/utility/logger.hpp>


namespace demo_game::detail {
    ve::gfx::texture_manager<texture_atlas_t>& get_texture_manager(void) {
        return *game::get_texture_manager();
    }
}