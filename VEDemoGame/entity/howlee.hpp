#pragma once

#include <VEDemoGame/game.hpp>

#include <VoxelEngine/ecs/ecs.hpp>
#include <VoxelEngine/graphics/graphics.hpp>
#include <VoxelEngine/utility/io/paths.hpp>


namespace demo_game {
    inline ve::mesh_component make_howlee_mesh(void) {
        using vertex = ve::gfx::vertex_types::texture_vertex_3d;


        auto buffer  = ve::gfxapi::indexed_vertex_buffer<vertex, ve::u32>::create();
        auto texture = game::get_texture_manager()->get_or_load(ve::io::paths::PATH_ENTITY_TEXTURES / "howlee.png");


        constexpr std::array pos_deltas {
            ve::vec3f { -0.5, 0, 0 },
            ve::vec3f { -0.5, 1, 0 },
            ve::vec3f { +0.5, 0, 0 },
            ve::vec3f { +0.5, 1, 0 }
        };

        constexpr std::array uv_deltas {
            ve::vec2f { 0, 1 },
            ve::vec2f { 0, 0 },
            ve::vec2f { 1, 1 },
            ve::vec2f { 1, 0 }
        };


        std::vector<vertex> vertices;
        for (const auto& [pos, uv] : ve::views::zip(pos_deltas, uv_deltas)) {
            vertices.emplace_back(vertex {
                .position      = pos,
                .uv            = texture.uv + (texture.wh * uv),
                .texture_index = texture.binding
            });
        }

        std::vector<ve::u32> indices = { 0, 2, 3, 0, 3, 1 };


        buffer->store_vertices(vertices);
        buffer->store_indices(indices);

        return ve::mesh_component { std::move(buffer) };
    }



    class howlee : public ve::static_entity {
    public:
        explicit howlee(ve::registry& registry) : ve::static_entity(registry) {
            // TODO: Replace this with instanced rendering once it is supported.
            this->mesh = make_howlee_mesh();
        }


        void VE_COMPONENT_FN(update)(ve::nanoseconds dt) {
            float seconds = float(dt.count()) / 1e9;
            transform.move(ve::vec3f { 0.0f, 0.1f * seconds, 0.0f });
        }


        ve::transform_component VE_COMPONENT(transform) = ve::transform_component { };
        ve::mesh_component VE_COMPONENT(mesh) = ve::mesh_component { };
    };
}