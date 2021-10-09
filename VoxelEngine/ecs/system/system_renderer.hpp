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
    struct dont_render_by_default_tag {};


    template <
        // Set of components which should be transformed into uniforms for each mesh.
        // Component classes provided here should implement ve::gfx::uniform_convertible.
        // Note that entities that lack these components will be excluded from the renderer.
        meta::pack_of_types ComponentUniforms = meta::pack<transform_component>,
        // Tags can be used to split meshes across different renderers.
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<dont_render_by_default_tag>
    > requires (
        ComponentUniforms::all([] <typename U> () { return requires { typename U::uniform_convertible_tag; }; })
    ) class system_renderer : public system<
        system_renderer<ComponentUniforms, RequiredTags, ExcludedTags>,
        typename RequiredTags
            ::template append_pack<ComponentUniforms>
            ::template append<mesh_component>
            ::unique,
        ExcludedTags
    > {
    public:
        explicit system_renderer(shared<gfxapi::pipeline> pipeline, u16 priority = priority::LOW) :
            priority(priority), pipeline(std::move(pipeline))
        {}


        u16 get_priority(void) const {
            return priority;
        }


        void update(registry& owner, view_type view, nanoseconds dt) {
            // TODO: Prevent having to do this copy each update, allow the pipeline to accept the view or something similar?
            static thread_local std::vector<const gfxapi::vertex_buffer*> buffers;
            auto clear_on_exit = raii_function { no_op, [&] { buffers.clear(); } };

            for (auto entity : view) {
                const auto& mesh = view.template get<mesh_component>(entity);
                VE_DEBUG_ASSERT(mesh.buffer, "Attempt to render uninitialized mesh component.");

                [&] <typename... Components> (meta::pack<Components...>) {
                    ([&] (const auto& cmp) {
                        // TODO: Cache these and use set_uniform_producer?
                        mesh.buffer->set_uniform_value(cmp);
                    }(view.template get<Components>(entity)), ...);
                }(ComponentUniforms{});

                buffers.push_back(mesh.buffer.get());
            }

            pipeline->draw(buffers, ctx);
        }

    private:
        u16 priority;
        shared<gfxapi::pipeline> pipeline;
        gfxapi::render_context ctx;

    public:
        VE_GET_SET_CREF(pipeline);
    };
}