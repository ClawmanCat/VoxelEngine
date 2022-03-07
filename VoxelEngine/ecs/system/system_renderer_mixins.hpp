#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system_mixin.hpp>
#include <VoxelEngine/ecs/system/system_utils.hpp>
#include <VoxelEngine/ecs/component/light_component.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>
#include <VoxelEngine/graphics/lighting/light_source.hpp>
#include <VoxelEngine/utility/color.hpp>
#include <VoxelEngine/utility/traits/bind.hpp>


namespace ve::renderer_mixins {
    // Mixin to add support for light sources to a system_renderer.
    // Note: this mixin may use separate include/exclude lists from its system,
    // since it may be desired to account for light sources that are not directly rendered themselves.
    template <typename System, meta::pack_of_types Included, meta::pack_of_types Excluded> class lighting_mixin : public system_mixin<
        lighting_mixin<System, Included, Excluded>,
        System,
        meta::pack_ops::merge_all<transform_component, light_component, Included, Excluded>
    > {
    public:
        using included_components = meta::pack_ops::merge_all<transform_component, light_component, Included>;
        using excluded_components = Excluded;
        using view_type           = component_view_type<included_components, excluded_components>;


        lighting_mixin(void) = default;


        template <typename Component> constexpr static u8 access_mode_for_component(void) {
            return (u8) system_access_mode::READ_CMP;
        }

        constexpr static bool has_unsafe_side_effects(void) {
            return false;
        }


        template <typename View>
        void before_system_update(System& self, registry& owner, View view, nanoseconds dt) {
            data.num_populated_lights = 0;

            auto emissive_entity_view = owner.template view_pack<included_components, excluded_components>();
            for (auto entt : emissive_entity_view) {
                const auto& [transform, light] = emissive_entity_view.template get<transform_component, light_component>(entt);

                data.lights[data.num_populated_lights++] = gfx::light_source {
                    .position    = transform.position,
                    .radiance    = light.radiance,
                    .attenuation = light.attenuation
                };
            }

            self.get_pipeline()->template set_uniform_value<gfx::lighting_data<>>(lighting_uniform_name, data);
        }


        void set_ambient_light(vec3f light) { data.ambient_light = light; }
        vec3f get_ambient_light(void) const { return data.ambient_light; }
    private:
        gfx::lighting_data<> data = gfx::lighting_data<> { .ambient_light = normalize_color(colors::WHITE) };
        std::string lighting_uniform_name = "U_Lighting";

    public:
        VE_GET_SET_CREF(lighting_uniform_name);
    };


    template <meta::pack_of_types Included = meta::pack<>, meta::pack_of_types Excluded = meta::pack<>>
    struct lighting_mixin_for {
        template <typename System> using type = lighting_mixin<System, Included, Excluded>;
    };
}