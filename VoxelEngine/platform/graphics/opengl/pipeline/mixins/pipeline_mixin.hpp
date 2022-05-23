#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/platform/graphics/opengl/pipeline/pipeline.hpp>


namespace ve::gfx::opengl {
    template <typename Derived, typename Pipeline>
    class pipeline_mixin {
    public:
        // Unlike the constructor, this method is called after the pipeline is completely constructed,
        // so it is safe to call virtual methods etc.
        void on_construct(void) {
            VE_MAYBE_CRTP_CALL(Derived, on_construct);
        }
    protected:
        Pipeline& get_pipeline(void) { return *static_cast<Pipeline*>(this); }
        const Pipeline& get_pipeline(void) const { return *static_cast<const Pipeline*>(this); }

        // Friend access.
        void pipeline_assert_has_ecs_mixins(const render_context& ctx, const std::vector<std::string_view>& mixins) const {
            get_pipeline().assert_has_ecs_mixins(ctx, mixins);
        }

        // Friend access.
        template <typename Mixin> const Mixin* pipeline_get_ecs_mixin(const render_context& ctx, std::string_view name) {
            return get_pipeline().template get_ecs_mixin<Mixin>(ctx, name);
        }
    };
}