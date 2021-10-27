#pragma once

#include <VoxelEngine/core/core.hpp>


namespace demo_game {
    // TODO: Fix empty components not working. This may be an issue with ENTT itself.
    struct render_tag_simple { ve::u16 nonempty = 0; };
    struct render_tag_pbr    { ve::u16 nonempty = 0; };
}