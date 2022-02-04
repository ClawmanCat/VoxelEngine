#pragma once

#include <VEDemoGame/component/render_tag.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>


namespace demo_game {
    class howlee : public ve::static_entity {
    public:
        using ve::static_entity::static_entity;
        ve_rt_move_only(howlee);


        struct entity_howlee_tag {};
        entity_howlee_tag VE_COMPONENT(tag) = entity_howlee_tag { };

        static void remote_initializer(ve::registry& owner, entt::entity entity, const entity_howlee_tag& cmp) {
            auto texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / "howlee.png");
            auto buffer  = ve::gfx::textured_quad(ve::vec2f { 1.0f }, texture);

            owner.set_component(entity, ve::mesh_component { std::move(buffer) });
            owner.set_component(entity, simple_render_tag { });
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