#pragma once

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>


namespace demo_game {
    class howlee : public ve::static_entity {
    public:
        using ve::static_entity::static_entity;
        ve_rt_move_only(howlee);


        static ve::mesh_component make_mesh(void) {
            auto texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / "howlee.png");
            auto buffer  = ve::gfx::textured_quad(ve::vec2f { 1.0f }, texture);

            return ve::mesh_component { std::move(buffer) };
        }


        void VE_COMPONENT_FN(update)(ve::nanoseconds dt) {
            t += float(dt.count()) / 1e9f;
            motion.linear_velocity.y = std::sin(t);
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::motion_component VE_COMPONENT(motion) = ve::motion_component { };
        float t = 0;
    };
}