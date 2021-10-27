#pragma once


namespace ve::gfx {
    // This is not ideal, but it is not possible to forward declare an alias,
    // and since this is the pre-include header, it is not possible to include them directly.
    template <typename Atlas> class texture_manager;
    template <typename Atlas> class generative_texture_atlas;
    class aligned_texture_atlas;
}


namespace demo_game {
    namespace detail {
        using texture_atlas_t = ve::gfx::generative_texture_atlas<ve::gfx::aligned_texture_atlas>;

        extern ve::gfx::texture_manager<texture_atlas_t>& get_texture_manager(void);
    }


    // Base provides default values for settings that are not overloaded.
    template <typename Base> struct voxel_settings : Base {
        using texture_atlas_t = detail::texture_atlas_t;

        // This is the texture atlas that will be used to store textures for tiles.
        static ve::gfx::texture_manager<texture_atlas_t>& get_texture_manager(void) {
            return detail::get_texture_manager();
        }
    };
}


#define VE_OVERLOADED_VOXEL_SETTINGS demo_game::voxel_settings