#pragma once

#include <VEDemoGame/game.hpp>
#include <VEDemoGame/component/render_tag.hpp>
#include <VEDemoGame/entity/player.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


namespace demo_game {
    // Yes, this isn't the "proper" way to implement a skybox but it simplifies things significantly.
    constexpr float skybox_distance = 1000.0f;


    class skybox : public ve::static_entity {
    public:
        explicit skybox(ve::registry& registry, class player* player) : ve::static_entity(registry), player(player) {
            auto tex = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / "he_knows.png");
            this->mesh.buffer = ve::gfx::textured_cube(ve::vec3f { skybox_distance }, tex);
        }


        void VE_COMPONENT_FN(update)(ve::nanoseconds dt) {
            transform.position = player->transform.position;
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::mesh_component VE_COMPONENT(mesh) = ve::mesh_component { };
        render_tag_simple VE_COMPONENT(render_tag) = render_tag_simple { };

    private:
        class player* player;
    };
}