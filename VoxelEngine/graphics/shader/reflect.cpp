#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/utility/functional.hpp>


namespace ve::gfx::reflect {
    shader_reflection generate_reflection(std::string name, const vec_map<const gfxapi::shader_stage*, spirv_blob>& stages) {
        VE_DEBUG_ASSERT(!stages.empty(), "Shader must consist of at least one stage.");

        const auto* pipeline = stages.begin()->first->pipeline;
        VE_DEBUG_ASSERT(
            ranges::all_of(stages | views::keys | views::indirect, equal_on(&gfxapi::shader_stage::pipeline, pipeline)),
            "All shader stages must be part of the same pipeline."
        );


        // Maps from fields in ShaderResources to fields in stage.
        constexpr std::array resource_map {
            std::pair { &spirv_cross::ShaderResources::stage_inputs,          &stage::inputs          },
            std::pair { &spirv_cross::ShaderResources::stage_outputs,         &stage::outputs         },
            std::pair { &spirv_cross::ShaderResources::uniform_buffers,       &stage::uniform_buffers },
            std::pair { &spirv_cross::ShaderResources::push_constant_buffers, &stage::push_constants  }
        };


        shader_reflection result { .name = std::move(name), .pipeline = pipeline };

        for (const auto& [stage_info, blob] : stages) {
            stage stage_result;

            spirv_cross::Compiler compiler { blob };
            spirv_cross::ShaderResources resources = compiler.get_shader_resources();

            for (auto [src, dst] : resource_map) {
                for (const auto& resource : resources.*src) {
                    (stage_result.*dst).push_back(attribute {
                        .name     = resource.name,
                        .type     = compiler.get_type(resource.type_id),
                        .location = compiler.get_decoration(resource.id, spv::DecorationLocation),
                        .binding  = compiler.get_decoration(resource.id, spv::DecorationBinding)
                    });
                }
            }

            result.stages.emplace(stage_info, std::move(stage_result));
        }


        return result;
    }
}