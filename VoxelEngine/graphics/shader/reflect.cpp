#include <VoxelEngine/graphics/shader/reflect.hpp>
#include <VoxelEngine/utility/functional.hpp>


namespace ve::gfx::reflect {
    shader_object reflect_object(
        const spirv_cross::Compiler& compiler,
        const std::string& name,
        const spirv_cross::SPIRType& type,
        std::size_t size,
        std::size_t offset,
        const glsl_object_source& source
    ) {
        shader_object object {
            .name              = name,
            .type              = object_type(type, compiler, source),
            .struct_size       = size,
            .offset_in_parent  = offset
        };


        if (type.basetype == spirv_cross::SPIRType::Struct) {
            VE_ASSERT(
                !std::holds_alternative<glsl_source_other_tag>(source),
                "Cannot reflect over struct members for struct not contained within UBO or SSBO."
            );


            for (std::size_t i = 0; i < type.member_types.size(); ++i) {
                std::size_t member_offset = 0;
                if (std::holds_alternative<glsl_source_ubo_member>(source)) {
                    member_offset = compiler.type_struct_member_offset(type, i);
                }

                object.members.push_back(reflect_object(
                    compiler,
                    compiler.get_member_name(type.self, i),
                    compiler.get_type(type.member_types[i]),
                    compiler.get_declared_struct_member_size(type, i),
                    member_offset,
                    glsl_source_ubo_member { .parent = type, .index = i }
                ));
            }
        }

        return object;
    }




    stage generate_stage_reflection(const gfxapi::shader_stage* stage, const spirv_blob& spirv) {
        // Maps from fields in ShaderResources to fields in stage.
        constexpr std::array resource_map {
            std::pair { &spirv_cross::ShaderResources::stage_inputs,          &stage::inputs          },
            std::pair { &spirv_cross::ShaderResources::stage_outputs,         &stage::outputs         },
            std::pair { &spirv_cross::ShaderResources::uniform_buffers,       &stage::uniform_buffers },
            std::pair { &spirv_cross::ShaderResources::storage_buffers,       &stage::storage_buffers },
            std::pair { &spirv_cross::ShaderResources::push_constant_buffers, &stage::push_constants  },
            std::pair { &spirv_cross::ShaderResources::sampled_images,        &stage::samplers        }
        };


        struct stage result;
        spirv_cross::CompilerGLSL compiler { spirv };
        spirv_cross::ShaderResources resources = compiler.get_shader_resources();

        for (auto [src, dst] : resource_map) {
            for (const auto& resource : resources.*src) {
                const auto& type = compiler.get_type(resource.type_id);


                glsl_object_source source = glsl_source_other_tag { };
                if (src == &spirv_cross::ShaderResources::uniform_buffers || src == &spirv_cross::ShaderResources::storage_buffers) {
                    source = glsl_source_ubo_tag { };
                }


                std::size_t struct_size = 0;
                if (type.basetype == spirv_cross::SPIRType::Struct) {
                    try {
                        struct_size = compiler.get_declared_struct_size(type);
                    } catch (...) {
                        // Interface blocks have no declared struct size. Just skip them for now.
                        // TODO: Find a way to reflect over interface blocks.
                        continue;
                    }
                }


                auto& attrib = (result.*dst).emplace_back(attribute {
                    reflect_object(compiler, resource.name, type, struct_size, 0, source),
                    compiler.get_decoration(resource.id, spv::DecorationLocation),
                    compiler.get_decoration(resource.id, spv::DecorationBinding)
                });
            }
        }

        return result;
    }




    shader_reflection generate_reflection(std::string name, const vec_map<const gfxapi::shader_stage*, spirv_blob>& stages) {
        VE_ASSERT(!stages.empty(), "Shader must consist of at least one stage.");

        for (const auto& stage_property : { &gfxapi::shader_stage::first, &gfxapi::shader_stage::last }) {
            VE_ASSERT(
                ranges::contains(stages | views::keys | views::indirect | views::transform(stage_property), true),
                "Shader must have an input and an output stage."
            );
        }

        const auto* pipeline = stages.begin()->first->pipeline;
        VE_ASSERT(
            ranges::all_of(stages | views::keys | views::indirect, equal_on(&gfxapi::shader_stage::pipeline, pipeline)),
            "All shader stages must be part of the same pipeline."
        );


        shader_reflection result { .name = std::move(name), .pipeline = pipeline };

        for (const auto& [stage_info, blob] : stages) {
            result.stages.emplace(stage_info, generate_stage_reflection(stage_info, blob));
        }

        return result;
    }
}