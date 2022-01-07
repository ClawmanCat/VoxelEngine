#include <VEDemoGame/component/render_tags.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/engine.hpp>


namespace demo_game {
    class howlee : public ve::static_entity {
    public:
        struct remote_init_tag {};


        static ve::mesh_component make_mesh(void) {
            auto texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / "howlee.png");
            auto buffer  = ve::gfx::textured_quad(ve::vec2f { 1.0f }, texture);

            return ve::mesh_component { std::move(buffer) };
        }


        static void remote_initializer(entt::entity entity, ve::registry& owner, const remote_init_tag& tag) {
            owner.set_component(entity, make_mesh());
            owner.set_component(entity, simple_render_tag { });
        }


        using static_entity::static_entity;


        void VE_COMPONENT_FN(update)(ve::nanoseconds dt) {
            motion.linear_velocity.y = std::sin(float(ve::engine::get_tick_count()) / 10.0f);
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::motion_component VE_COMPONENT(motion) = ve::motion_component { };
        ve::remote_init_component VE_COMPONENT(tag) = ve::remote_init_for<remote_init_tag>();
    };
}