#include <VoxelEngine/graphics/shader/preprocessor/constant_reflection.hpp>


namespace ve::gfx {
    constexpr inline ctll::fixed_string regex = R"RGX(layout\s*\(\s*constant_id\s*=\s*(\d+)\s*\)\s+const\s+(\S+)\s+(\S+))RGX";


    void constant_reflection_preprocessor::operator()(std::string& src, arbitrary_storage& context) const {
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


    reflect::attribute constant_reflection_preprocessor::generate_attribute(std::string_view binding, std::string_view type_string, std::string_view name) {
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


    reflect::object_type::base_type_t constant_reflection_preprocessor::get_base_type(std::string_view type) {
        using BT = reflect::object_type::base_type_t;


        // No graphics APIs currently allow specialization constants of non-scalar type,
        // so we don't have to worry about tensors / structs / arrays / etc.
        if (type == "int")                       return BT::INT;
        if (type == "uint"  || type == "bool"  ) return BT::UINT;
        if (type == "float" || type == "double") return BT::FLOAT;


        VE_ASSERT(false, "Illegal specialization constant type: ", type);
        VE_UNREACHABLE;
    }
}