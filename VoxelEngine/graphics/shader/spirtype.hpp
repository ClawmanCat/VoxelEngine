#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/tuple_foreach.hpp>
#include <VoxelEngine/utility/traits/is_std_array.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>

#include <spirv_cross/spirv_common.hpp>


namespace ve::gfx::reflect {
    using primitive_t = spirv_cross::SPIRType;


    // Generates a dummy SPIRType for the type T so T can be checked for compatibility as a shader input.
    // TODO: Support trivial struct types to simplify UBO usage.
    // TODO: Support sampler types.
    template <typename T> requires (
        std::is_scalar_v<T>         ||
        std::is_bounded_array_v<T>  ||
        meta::is_std_array_v<T>     ||
        meta::glm_traits<T>::is_glm
    ) inline primitive_t spirtype_for(void) {
        auto push_front = [] (auto& v, auto&& elem) -> decltype(auto) { v.insert(v.begin(), fwd(elem)); return v; };

        using BT     = spirv_cross::SPIRType::BaseType;
        using Traits = meta::glm_traits<T>;

        // Note: sorted by size. type = types[lsb(sizeof(T))].
        constexpr std::array signed_int_types   { BT::SByte,   BT::Short,  BT::Int,   BT::Int64  };
        constexpr std::array unsigned_int_types { BT::UByte,   BT::UShort, BT::UInt,  BT::UInt64 };
        constexpr std::array float_types        { BT::Unknown, BT::Half,   BT::Float, BT::Double };


        primitive_t result;

        // Simple scalar types.
        if constexpr (std::is_scalar_v<T>) {
            if      constexpr (std::is_floating_point_v<T>) result.basetype = float_types[lsb(sizeof(T))];
            else if constexpr (std::is_signed_v<T>)         result.basetype = signed_int_types[lsb(sizeof(T))];
            else if constexpr (std::is_unsigned_v<T>)       result.basetype = unsigned_int_types[lsb(sizeof(T))];
        }

        else if constexpr (Traits::is_glm) {
            result.basetype = spirtype_for<typename Traits::value_type>().basetype;
            result.vecsize  = Traits::num_rows;
            result.columns  = Traits::num_cols;
        }

        else if constexpr (meta::is_std_array_v<T>) {
            auto element_spirtype = spirtype_for<typename T::value_type>();

            result.basetype           = element_spirtype.basetype;
            result.array              = push_front(element_spirtype.array, std::tuple_size_v<T>);
            // Since we're not reflecting a shader, specialization constants are not applicable here.
            result.array_size_literal = push_front(element_spirtype.array_size_literal, true);
        }

        else if constexpr (std::is_bounded_array_v<T>) {
            using E = std::remove_cvref_t<decltype(*std::declval<T>())>;
            auto element_spirtype = spirtype_for<E>();

            result.basetype           = element_spirtype.basetype;
            result.array              = push_front(element_spirtype.array, sizeof(T) / sizeof(E));
            // Since we're not reflecting a shader, specialization constants are not applicable here.
            result.array_size_literal = push_front(element_spirtype.array_size_literal, true);
        }


        return result;
    }


    inline std::strong_ordering compare_spirtypes(const primitive_t& a, const primitive_t& b) {
        std::strong_ordering result = std::strong_ordering::equal;

        // Can't use Boost PFR here since SPIRType is a polymorphic class.
        // Note: only fields by spirtype_for<T> are checked.
        std::tuple fields {
            &primitive_t::basetype,
            &primitive_t::width,
            &primitive_t::vecsize,
            &primitive_t::columns
        };

        tuple_foreach(fields, [&](auto field) {
            result = (a.*field <=> b.*field);
            return result != std::strong_ordering::equal;
        });


        for (const auto& [af, bf] : views::zip(a.array, b.array)) {
            result = (af <=> bf);
            if (result != std::strong_ordering::equal) break;
        }


        return result;
    }
}