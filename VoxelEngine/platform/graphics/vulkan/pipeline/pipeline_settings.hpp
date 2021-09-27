#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/tuple_foreach.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan {
    struct pipeline_settings {
        enum class topology_t {
            POINTS         = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
            LINES          = VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
            TRIANGLES      = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            TRIANGLE_FAN   = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
            TRIANGLE_STRIP = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
            PATCHES        = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST
        } topology = topology_t::TRIANGLES;


        enum class polygon_mode_t {
            SURFACE  = VK_POLYGON_MODE_FILL,
            EDGES    = VK_POLYGON_MODE_LINE,
            VERTICES = VK_POLYGON_MODE_POINT
        } polygon_mode = polygon_mode_t::SURFACE;


        enum class cull_mode_t {
            NO_CULLING = VK_CULL_MODE_NONE,
            CULL_FRONT = VK_CULL_MODE_FRONT_BIT,
            CULL_BACK  = VK_CULL_MODE_BACK_BIT,
            CULL_BOTH  = VK_CULL_MODE_FRONT_AND_BACK
        } cull_mode = cull_mode_t::CULL_BACK;


        enum class cull_direction_t {
            CLOCKWISE         = VK_FRONT_FACE_CLOCKWISE,
            COUNTER_CLOCKWISE = VK_FRONT_FACE_COUNTER_CLOCKWISE
        } cull_direction = cull_direction_t::CLOCKWISE;


        float line_width   = 1.0f;
        bool  enable_depth = true;
        bool  clamp_depth  = true;

        vec4f clear_color = { 0.65f, 0.65f, 0.65f, 1.00f };


        // Changes to these fields don't require the pipeline to be recreated.
        constexpr static inline std::tuple dynamic_fields {
            &pipeline_settings::topology,
            &pipeline_settings::cull_mode,
            &pipeline_settings::cull_direction,
            &pipeline_settings::line_width,
            &pipeline_settings::enable_depth,
            &pipeline_settings::clear_color
        };

        // Changes to these fields cannot be handled through dynamic state and require separate pipelines.
        constexpr static inline std::tuple non_dynamic_fields {
            &pipeline_settings::polygon_mode,
            &pipeline_settings::clamp_depth
        };


        // Hashes only on the fields that require separate pipelines.
        struct dynamic_compatibility_hasher {
            constexpr std::size_t operator()(const pipeline_settings& settings) const {
                std::size_t hash = 0;

                tuple_foreach(
                    pipeline_settings::non_dynamic_fields,
                    [&](const auto& field) { boost::hash_combine(hash, settings.*field); }
                );

                return hash;
            }
        };

        // Compares only on the fields that require separate pipelines.
        struct dynamic_compatibility_equality {
            constexpr bool operator()(const pipeline_settings& a, const pipeline_settings& b) const {
                bool equal = true;

                tuple_foreach(
                    pipeline_settings::non_dynamic_fields,
                    [&](const auto& field) { equal &= (a.*field == b.*field); }
                );

                return equal;
            }
        };
    };



    static_assert(
        std::tuple_size_v<decltype(pipeline_settings::dynamic_fields)>     +
        std::tuple_size_v<decltype(pipeline_settings::non_dynamic_fields)> ==
        boost::pfr::tuple_size_v<pipeline_settings>
    );
}