#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/utility/functional.hpp>


namespace ve::gfx::reflect {
    inline shader_object reflect_object(const spirv_cross::Compiler& compiler, const std::string& name, const spirv_cross::TypeID& type) {
        auto spir_type = compiler.get_type(type);

        shader_object object {
            .name = name,
            .type = spir_type
        };

        if (spir_type.basetype == primitive_t::Struct) {
            for (std::size_t i = 0; i < spir_type.member_types.size(); ++i) {
                object.members.push_back(reflect_object(
                    compiler,
                    compiler.get_member_name(spir_type.self, i),
                    spir_type.member_types[i]
                ));
            }
        }

        return object;
    }


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
            std::pair { &spirv_cross::ShaderResources::push_constant_buffers, &stage::push_constants  },
            std::pair { &spirv_cross::ShaderResources::sampled_images,        &stage::samplers        }
        };


        shader_reflection result { .name = std::move(name), .pipeline = pipeline };

        for (const auto& [stage_info, blob] : stages) {
            stage stage_result;

            spirv_cross::Compiler compiler { blob };
            spirv_cross::ShaderResources resources = compiler.get_shader_resources();

            for (auto [src, dst] : resource_map) {
                for (const auto& resource : resources.*src) {
                    auto& attrib = (stage_result.*dst).emplace_back(attribute {
                        reflect_object(compiler, resource.name, resource.type_id),
                        compiler.get_decoration(resource.id, spv::DecorationLocation),
                        compiler.get_decoration(resource.id, spv::DecorationBinding)
                    });
                }
            }

            result.stages.emplace(stage_info, std::move(stage_result));
        }


        return result;
    }
}