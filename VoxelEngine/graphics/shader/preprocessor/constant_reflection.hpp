#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/shader/preprocessor/shader_preprocessor.hpp>
#include <VoxelEngine/graphics/shader/reflect.hpp>

#include <VoxelEngine/platform/graphics/graphics_includer.hpp>
#include VE_GFX_HEADER(shader/stage.hpp)

#include <ctre.hpp>


namespace ve::gfx {
    // Perform reflection on any specialization constants in the shader, since SPIRV-Reflect currently doesn't support it.
    class constant_reflection_preprocessor : public shader_preprocessor {
    public:
        constexpr static inline ctll::fixed_string regex = R"RGX(layout\s*\(\s*constant_id\s*=\s*(\d+)\s*\)\s+const\s+(\S+)\s+(\S+))RGX"    ;
        using attribute_map = hash_map<const gfxapi::shader_stage*, std::vector<reflect::attribute>>;


        using shader_preprocessor::shader_preprocessor;


        void operator()(std::string& src, arbitrary_storage& context) const override {
            // Regex for finding specialization constants. Captures the following:
            // 01: the binding index of the constant.
            // 02: the variable type of the constant.
            // 03: the name of the constant.


            auto& attributes  = context.template get_or_create_object<attribute_map>("ve.constant_reflect.attribs");
            const auto* stage = context.template get_object<const gfxapi::shader_stage*>("ve.shader_stage");

            for (const auto& [match, binding, type, name] : ctre::range<regex>(src)) {
                auto reflection = generate_attribute(binding, type, name);
                if (!ranges::contains(attributes[stage], reflection.binding, &reflect::attribute::binding)) attributes[stage].push_back(reflection);
            }
        }
    private:
        static reflect::attribute generate_attribute(std::string_view binding, std::string_view type_string, std::string_view name) {
            reflect::object_type type;
            type.base_type     = get_base_type(type_string);
            type.array_extents = {};
            type.base_size     = 4;
            type.columns       = 1;
            type.rows          = 1;


            reflect::shader_object object {
                .name = std::string { name },
                .type = std::move(type)
            };

            std::string binding_string { binding };
            return reflect::attribute { std::move(object), 0, (std::size_t) std::stoi(binding_string) };
        }


        static reflect::object_type::base_type_t get_base_type(std::string_view type) {
            using BT = reflect::object_type::base_type_t;


            // No graphics APIs currently allow specialization constants of non-scalar type,
            // so we don't have to worry about tensors / structs / arrays / etc.
            if (type == "int")                       return BT::INT;
            if (type == "uint"  || type == "bool"  ) return BT::UINT;
            if (type == "float" || type == "double") return BT::FLOAT;


            VE_ASSERT(false, "Illegal specialization constant type: ", type);
            VE_UNREACHABLE;
        }
    };
}