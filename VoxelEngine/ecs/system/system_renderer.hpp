#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/mesh_component.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(pipeline/pipeline.hpp)
#include VE_GFX_HEADER(context/render_context.hpp)


namespace ve {
    template <
        // Set of components which should be transformed into uniforms for each mesh.
        // Component classes provided here should implement ve::gfx::uniform_convertible.
        // Note that entities that lack these components will be excluded from the renderer.
        meta::pack_of_types ComponentUniforms = meta::pack<transform_component>,
        // Tags can be used to split meshes across different renderers.
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<>,
        template <typename System> typename... Mixins
    > requires (
        ComponentUniforms::all([] <typename U> () { return requires { typename U::uniform_convertible_tag; }; })
    ) class system_renderer : public system<
        system_renderer<ComponentUniforms, RequiredTags, ExcludedTags, Mixins...>,
        meta::pack_ops::merge_all<ComponentUniforms, RequiredTags, mesh_component>,
        ExcludedTags,
        deduce_component_access,
        Mixins...
    > {
    public:
        explicit system_renderer(shared<gfxapi::pipeline> pipeline, u16 priority = priority::LOW) :
            priority(priority), pipeline(std::move(pipeline))
        {}


        u16 get_priority(void) const {
            return priority;
        }


        template <typename Component> constexpr static u8 access_mode_for_component(void) {
            return (u8) system_access_mode::READ_CMP;
        }


        void on_system_update(registry& owner, view_type view, nanoseconds dt) {
            VE_PROFILE_FN();


            // TODO: Prevent having to do this copy each update, allow the pipeline to accept the view or something similar?
            static thread_local gfxapi::pipeline_draw_data data { .ctx = &ctx };
            auto clear_on_exit = raii_function { no_op, [&] { data.buffers.clear(); } };


            for (auto entity : view) {
                const auto& mesh = view.template get<mesh_component>(entity);
                VE_DEBUG_ASSERT(mesh.buffer, "Attempt to render uninitialized mesh component.");

                ComponentUniforms::foreach([&] <typename Component> {
                    mesh.buffer->set_uniform_value(view.template get<Component>(entity));
                });

                data.buffers.push_back(mesh.buffer.get());
            }

            pipeline->draw(data);
        }

    private:
        u16 priority;
        shared<gfxapi::pipeline> pipeline;
        gfxapi::render_context ctx;

    public:
        VE_GET_SET_VAL(pipeline);
        VE_GET_MREF(ctx);
    };
}