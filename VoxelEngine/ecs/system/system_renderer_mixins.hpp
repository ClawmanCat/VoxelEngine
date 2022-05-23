#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/registry.hpp>
#include <VoxelEngine/ecs/system/system_mixin.hpp>
#include <VoxelEngine/ecs/system/system_utils.hpp>
#include <VoxelEngine/ecs/component/light_component.hpp>
#include <VoxelEngine/ecs/component/transform_component.hpp>
#include <VoxelEngine/graphics/lighting/light_source.hpp>
#include <VoxelEngine/graphics/lighting/bloom_data.hpp>
#include <VoxelEngine/graphics/lighting/gaussian_blur.hpp>
#include <VoxelEngine/utility/color.hpp>
#include <VoxelEngine/utility/traits/bind.hpp>


namespace ve::renderer_mixins {
    class render_mixin_base;
    using mixin_list_t = hash_map<std::string, const render_mixin_base*>;


    // Interface for renderer mixins. Pipeline can cast the mixin to this class without knowing the exact templated type.
    class render_mixin_base {
    public:
        explicit render_mixin_base(std::string mixin_name) : mixin_name(std::move(mixin_name)) {}
        virtual ~render_mixin_base(void) = default;

        VE_GET_CREF(mixin_name);
    protected:
        template <typename System> void attach_to_context(System& self) {
            // The mixin itself cannot be removed from the system, so we don't have to worry about that.
            // The system, and therefore the mixin can however be moved while they are not added to the registry,
            // so update the mixin address every time we get re-added.
            self.get_ctx()
                .template get_or_create_object<mixin_list_t>("ve.render_mixins")
                .insert_or_assign(mixin_name, this);
        }

    private:
        std::string mixin_name;
    };


    // Mixin to add support for light sources to a system_renderer.
    // Note: this mixin may use separate include/exclude lists from its system,
    // since it may be desired to account for light sources that are not directly rendered themselves.
    // TODO: Refactor the way in which data is passed from mixin to pipeline. The pipeline should not need to have knowledge of the mixin.
    template <typename System, meta::pack_of_types Included, meta::pack_of_types Excluded>
    class lighting_mixin :
        public render_mixin_base,
        public system_mixin<
            lighting_mixin<System, Included, Excluded>,
            System,
            meta::pack_ops::merge_all<transform_component, light_component, Included, Excluded>
        >
    {
    public:
        using included_components = meta::pack_ops::merge_all<transform_component, light_component, Included>;
        using excluded_components = Excluded;
        using view_type           = component_view_type<included_components, excluded_components>;


        lighting_mixin(void) : render_mixin_base("ve.lighting_mixin") {}


        template <typename Component> constexpr static u8 access_mode_for_component(void) {
            return (u8) system_access_mode::READ_CMP;
        }

        constexpr static bool has_unsafe_side_effects(void) {
            return false;
        }


        void before_system_added(System& self, registry& owner) {
            attach_to_context(self);
        }


        template <typename View>
        void before_system_update(System& self, registry& owner, View view, nanoseconds dt) {
            data.num_populated_lights = 0;

            // Convert all entities with light components to light sources.
            auto emissive_entity_view = owner.template view_pack<included_components, excluded_components>();
            for (auto entt : emissive_entity_view) {
                const auto& [transform, light] = emissive_entity_view.template get<transform_component, light_component>(entt);

                data.lights[data.num_populated_lights++] = gfx::light_source {
                    .position         = transform.position + vec3f { 0.0f, 0.5f, 0.0f },
                    .radiance         = light.radiance,
                    .attenuation      = light.attenuation
                };
            }

            self.get_pipeline()->template set_uniform_value<gfx::lighting_data<>>(lighting_uniform_name, data);
        }


        void set_ambient_light(vec3f value) { data.ambient_light = value; }
        vec3f get_ambient_light(void) const { return data.ambient_light; }

        void set_exposure(float value) { data.exposure = value; }
        float get_exposure(void) const { return data.exposure; }

        void set_emissivity_constant(float value) { data.emissivity_constant = value; }
        float get_emissivity_constant(void) const { return data.emissivity_constant; }
    private:
        using lighting_data_t = gfx::lighting_data<>;


        lighting_data_t data = lighting_data_t {
            .ambient_light       = normalize_color(colors::WHITE),
            .exposure            = 1.0f,
            .emissivity_constant = 5.0f
        };

        std::string lighting_uniform_name = "U_Lighting";

    public:
        VE_GET_SET_CREF(lighting_uniform_name);
    };


    template <meta::pack_of_types Included = meta::pack<>, meta::pack_of_types Excluded = meta::pack<>>
    struct lighting_mixin_for {
        template <typename System> using type = lighting_mixin<System, Included, Excluded>;
    };




    template <typename System> class bloom_mixin : public render_mixin_base, public system_mixin<bloom_mixin<System>, System> {
    public:
        bloom_mixin(void) : render_mixin_base("ve.bloom_mixin") {
            set_blur_weights(gfx::make_gaussian_weights(5, 2.0f));
        }


        void before_system_added(System& self, registry& owner) {
            attach_to_context(self);
        }


        template <typename View>
        void before_system_update(System& self, registry& owner, View view, nanoseconds dt) {
            self.get_pipeline()->template set_uniform_value<gfx::bloom_data>(bloom_uniform_name, bloom_data);
            self.get_pipeline()->template set_uniform_value<gfx::gaussian_blur_data<>>(blur_uniform_name, blur_data);

            self.get_ctx().template store_or_replace_object<u32>("ve.bloom_mixin.num_bloom_passes", num_bloom_passes);
            self.get_ctx().template store_or_replace_object<float>("ve.bloom_mixin.scale_factor", scale_factor);
        }


        void set_luma_conversion_weights(vec3f value) { bloom_data.luma_conversion_weights = value; }
        vec3f get_luma_conversion_weights(void) const { return bloom_data.luma_conversion_weights; }

        void set_bloom_intensity(float value) { bloom_data.bloom_intensity = value; }
        float get_bloom_intensity(void) const { return bloom_data.bloom_intensity; }

        void set_bloom_threshold(float value) { bloom_data.bloom_threshold = value; }
        float get_bloom_threshold(void) const { return bloom_data.bloom_threshold; }

        void set_blur_range(float value) { blur_data.range = value; }
        float get_blur_range(void) const { return blur_data.range; }


        void set_blur_weights(const std::vector<float>& weights) {
            constexpr auto limit = decltype(blur_data)::weight_count_limit;
            VE_DEBUG_ASSERT(weights.size() <= limit, "Too many weights provided for gaussian blur. The limit is ", limit, " weights.");

            ranges::copy(weights, blur_data.weights.begin());
            blur_data.populated_weights = weights.size();
        }

        std::vector<float> get_blur_weights(void) const {
            return { blur_data.weights.begin(), blur_data.weights.begin() + blur_data.populated_weights };
        }

    private:
        gfx::bloom_data bloom_data = gfx::bloom_data { };
        gfx::gaussian_blur_data<> blur_data = gfx::gaussian_blur_data<> { };
        u32 num_bloom_passes = 4;
        float scale_factor = 0.5f;

        std::string bloom_uniform_name = "U_BloomData";
        std::string blur_uniform_name = "U_GaussianData";

    public:
        VE_GET_SET_CREF(bloom_uniform_name);
        VE_GET_SET_CREF(blur_uniform_name);
        VE_GET_SET_VAL(num_bloom_passes);
        VE_GET_SET_VAL(scale_factor);
    };
}