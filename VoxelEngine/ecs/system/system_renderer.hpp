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
    namespace detail {
        template <std::size_t N> struct nth {
            template <typename... Ts> using type = typename meta::pack<typename Ts::template get<N>...>;
        };


        template <typename ComponentUniforms> using components_for_cu_pairs = typename ComponentUniforms
            ::template expand_inside<nth<0>::template type>;

        template <typename ComponentUniforms> using uniforms_for_cu_pairs = typename ComponentUniforms
            ::template expand_inside<nth<1>::template type>;
    }


    struct dont_render_by_default_tag {};


    template <
        // Set of components which should be transformed into uniforms for each mesh plus a transform method for each such component.
        // Transform methods should be classes implementing the uniform_component interface.
        meta::pack_of_types ComponentUniforms = meta::pack<
            meta::pack<transform_component, transform_to_uniform>
        >,
        // Tags can be used to split meshes across different renderers.
        meta::pack_of_types RequiredTags = meta::pack<>,
        meta::pack_of_types ExcludedTags = meta::pack<dont_render_by_default_tag>
    > class system_renderer : public system<
        system_renderer<RequiredTags, ExcludedTags>,
        typename RequiredTags
            ::template append_pack<detail::components_for_cu_pairs<ComponentUniforms>>
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


        void update(view_type view, nanoseconds dt) {
            using uniform_components = detail::components_for_cu_pairs<ComponentUniforms>;
            using uniform_producers  = detail::uniforms_for_cu_pairs<ComponentUniforms>;


            // TODO: Prevent having to do this copy each update, allow the pipeline to accept the view or something similar?
            static thread_local std::vector<const gfxapi::vertex_buffer*> buffers;
            auto clear_on_exit = raii_function { no_op, [&] { buffers.clear(); } };

            for (auto entity : view) {
                const auto& mesh = view.template get<mesh_component>(entity);

                [&] <typename... Components> (meta::pack<Components...>) {
                    [&] <typename... Producers> (meta::pack<Producers...>) {
                        ([&] <typename C, typename P> (const C& component, const P& producer) {
                            // TODO: Cache these and use set_uniform_producer.
                            mesh.buffer->template set_uniform_value<typename P::uniform_t>(
                                producer.name(component),
                                producer.value(component)
                            );
                        }(
                            view.template get<Components>(entity),
                            Producers{}
                        ), ...);
                    }(uniform_producers{});
                }(uniform_components{});

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