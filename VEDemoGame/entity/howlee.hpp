#pragma once

#include <VEDemoGame/game.hpp>
#include <VEDemoGame/component/render_tag.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


namespace demo_game {
    inline ve::mesh_component make_howlee_mesh(void) {
        auto texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / "howlee.png");
        auto buffer  = ve::gfx::textured_quad(ve::vec2f { 1.0f }, texture);

        return ve::mesh_component { std::move(buffer) };
    }



    class howlee : public ve::static_entity {
    public:
        explicit howlee(ve::registry& registry) : ve::static_entity(registry) {
            // TODO: Replace this with instanced rendering once it is supported.
            this->mesh = make_howlee_mesh();
        }


        void VE_COMPONENT_FN(update)(ve::nanoseconds dt) {
            transform.position.y += 0.1f * (float(dt.count()) / 1e9f);
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::mesh_component VE_COMPONENT(mesh) = ve::mesh_component { };
        render_tag_simple VE_COMPONENT(render_tag) = render_tag_simple { };
    };
}