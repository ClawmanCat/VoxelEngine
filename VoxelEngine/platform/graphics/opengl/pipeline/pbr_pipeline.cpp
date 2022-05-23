#include <VoxelEngine/platform/graphics/opengl/pipeline/pbr_pipeline.hpp>


namespace ve::gfx::opengl {
    void pbr_pipeline::draw(const pipeline_draw_data& data, overridable_function_tag) {
        // Can't have PBR without lighting information.
        const static std::vector<std::string_view> required_mixins { "ve.lighting_mixin" };
        assert_has_ecs_mixins(*data.ctx, required_mixins);

        // Same as draw data, but with the screen-filling quad as its only vertex buffer.
        pipeline_draw_data quad_data = draw_data_to_g_input(data);

        // Note: the target of the postprocessing pass is the final target, so don't clear that, in case other stuff is rendered to it as well.
        geometry_pass->get_target()->clear();
        lighting_pass->get_target()->clear();


        dispatch_event(pbr_pre_geometry_pass_event { .pipeline = this, .draw_data = &data });
        geometry_pass->draw(data);
        dispatch_event(pbr_post_geometry_pass_event { .pipeline = this, .draw_data = &data });

        dispatch_event(pbr_pre_lighting_pass_event { .pipeline = this, .draw_data = &data });
        lighting_pass->draw(quad_data);
        dispatch_event(pbr_post_lighting_pass_event { .pipeline = this, .draw_data = &data });

        dispatch_event(pbr_pre_postprocessing_pass_event { .pipeline = this, .draw_data = &data });
        postprocessing_pass->draw(quad_data);
        dispatch_event(pbr_post_postprocessing_pass_event { .pipeline = this, .draw_data = &data });
    }


    void pbr_pipeline::setup_pipeline(void) {
        VE_PROFILE_FN();


        dispatch_event(pbr_pipeline_pre_build_event { .pipeline = this });


        auto geometry_pass_shader = get_shader<vertex_types::material_vertex_3d>(
            "ve.pbr_geometry",
            "pipeline_pbr/pbr.vert.glsl", "pipeline_pbr/pbr_geometry.frag.glsl"
        );

        auto lighting_pass_shader = get_shader<vertex_types::no_vertex>(
            "ve.pbr_lighting",
            "pipeline_common/screen_quad.g_input.glsl", "pipeline_pbr/pbr_lighting.frag.glsl"
        );

        auto postprocessing_pass_shader = get_shader<vertex_types::no_vertex>(
            "ve.pbr_postprocessing",
            "pipeline_common/screen_quad.g_input.glsl", "pipeline_pbr/pbr_postprocessing.frag.glsl"
        );


        auto renderpass_defs = std::vector {
            renderpass_definition { geometry_pass_shader }
                | then([] (auto& pass) {
                    pass.add_depth_attachment("depth");
                    pass.add_color_attachments({ "position", "normal", "color", "material" });
                    pass.modify_attachment("position", [] (auto& a) { a.clear_value = vec4f { 0, 0, 0, 1 }; }); // position.z = depth
                }),

            renderpass_definition { lighting_pass_shader, name_transforms::add_prefix { "g_" } }
                | then([] (auto& pass) {
                    pass.add_color_attachments({ "position", "color" });
                    pass.modify_attachment("position", [] (auto& a) { a.clear_value = vec4f { 0, 0, 0, 1 }; }); // position.z = depth
                }),

            renderpass_definition { postprocessing_pass_shader, name_transforms::add_prefix { "l_" } }
                | then([] (auto& pass) {
                    pass.add_indirect_input(1, "position");
                    pass.add_indirect_input(1, "color");
                })
        };


        dispatch_event(pbr_pipeline_pre_build_stages_event { .pipeline = this, .renderpass_defs = &renderpass_defs });
        auto renderpasses = build_pipeline(renderpass_defs, get_target());

        auto renderpasses_base = renderpasses
            | views::transform([] (auto& ptr) { return std::static_pointer_cast<pipeline>(ptr); })
            | ranges::to<std::vector>;

        dispatch_event(pbr_pipeline_post_build_stages_event { .pipeline = this, .renderpasses = &renderpasses_base });


        for (const auto& ptr : renderpasses) pipeline_stages.push_back(ptr);
        move_into(std::move(renderpasses), geometry_pass, lighting_pass, postprocessing_pass);

        dispatch_event(pbr_pipeline_post_build_event { .pipeline = this });
    }
}