#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/value.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/utility/traits/is_std_array.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>

#include <spirv_cross/spirv_reflect.hpp>


namespace ve::gfx::reflect {
    struct glsl_source_ubo_tag {};
    struct glsl_source_other_tag {};
    struct glsl_source_ubo_member { spirv_cross::SPIRType parent; std::size_t index; };

    // Querying SPIRtype information works differently depending on where the object came from.
    using glsl_object_source = std::variant<
        glsl_source_ubo_tag,    // If the type is an UBO / SSBO, we can get the size from the type itself.
        glsl_source_ubo_member, // If the type is a member of an UBO / SSBO, we can get the size from the parent type.
        glsl_source_other_tag   // Otherwise the size information is not exposed and cannot be deduced.
    >;


    // Represents the type of an object. Can be used to compare both CPU- and GPU-side objects.
    // This is a replacement for spirv_cross::SPIRType, to make it usable with CPU types as well,
    // and aims to mostly expose an identical API, with exception of features not used in the engine.
    struct object_type {
        enum base_type_t { INT, UINT, FLOAT, STRUCT, SAMPLER } base_type;

        // Size of the base type in bytes, e.g. 4 for i32, 8 for i64, etc.
        std::size_t base_size;

        // Vector / Matrix types.
        std::size_t rows = 1, columns = 1;

        // Arrays can be multidimensional. In the case of scalars, this vector is empty.
        std::vector<std::size_t> array_extents;

        // Members of this type, if this is a struct.
        std::vector<object_type> struct_members;


        ve_rt_eq_comparable(object_type);
        ve_field_hashable(base_type, base_size, rows, columns, array_extents, struct_members);

        object_type(void) = default;


        // Construct an object type from a SPIRV reflected type.
        object_type(const spirv_cross::SPIRType& spirtype, const spirv_cross::Compiler& compiler, const glsl_object_source& source = glsl_source_other_tag { }) {
            using BT = spirv_cross::SPIRType::BaseType;

            constexpr std::array signed_int_types   { BT::SByte, BT::Short,  BT::Int,  BT::Int64  };
            constexpr std::array unsigned_int_types { BT::UByte, BT::UShort, BT::UInt, BT::UInt64 };
            constexpr std::array float_types        { BT::Half,  BT::Float,  BT::Double           };

            constexpr std::array int_sizes   { 1, 2, 4, 8 };
            constexpr std::array float_sizes { 2, 4, 8 };


            // Returns the index of the type in the array, or nullopt if the type is not in the array.
            auto index_of = [&] (const auto& src, BT type) -> std::optional<u32> {
                auto it = ranges::find(src, type);

                return (it == src.end())
                    ? std::nullopt
                    : std::optional<u32> { std::distance(src.begin(), it) };
            };


            if (auto index = index_of(signed_int_types, spirtype.basetype); index) {
                base_type = base_type_t::INT;
                base_size = int_sizes[*index];
            }

            else if (auto index = index_of(unsigned_int_types, spirtype.basetype); index) {
                base_type = base_type_t::UINT;
                base_size = int_sizes[*index];
            }

            else if (auto index = index_of(float_types, spirtype.basetype); index) {
                base_type = base_type_t::FLOAT;
                base_size = float_sizes[*index];
            }

            else {
                switch (spirtype.basetype) {
                    case BT::Struct:
                        base_type = base_type_t::STRUCT;

                        if (std::holds_alternative<glsl_source_ubo_tag>(source)) {
                            base_size = compiler.get_declared_struct_size(spirtype);
                        } else if (std::holds_alternative<glsl_source_other_tag>(source)) {
                            VE_ASSERT(false, "Cannot reflect over SPIRType of struct with non-exposed size.");
                        } else {
                            const auto& member = std::get<glsl_source_ubo_member>(source);
                            base_size = compiler.get_declared_struct_member_size(member.parent, member.index);
                        }

                        break;
                    case BT::Sampler:
                    case BT::SampledImage:
                        base_type = base_type_t::SAMPLER;
                        base_size = 0;
                        break;
                    default: VE_ASSERT(false, "Unsupported SPIRType for ve::gfx::reflect::object_type");
                }
            }


            array_extents = { spirtype.array.begin(), spirtype.array.end() };
            rows          = spirtype.vecsize;
            columns       = spirtype.columns;


            if (base_type == base_type_t::STRUCT) {
                for (const auto& [i, member_id] : spirtype.member_types | views::enumerate) {
                    struct_members.push_back(object_type {
                        compiler.get_type(member_id),
                        compiler,
                        glsl_source_ubo_member { spirtype, i }
                    });
                }
            }
        }


        // Construct an object type from a C++ type.
        template <typename T> explicit object_type(meta::type_wrapper<T>) {
            if constexpr (std::is_array_v<T> || meta::is_std_array_v<T>) {
                // If this is an array, the object type is the value type except it has an array extent.
                // Note: if the value type is also an array, the current extent simply goes before that of the value.
                (*this) = object_type(meta::type_wrapper<meta::array_value_type<T>>{});
                array_extents.insert(array_extents.begin(), meta::array_size<T>);
            }

            else if constexpr (meta::glm_traits<T>::is_glm) {
                // If this is a vector or matrix, the object type is the value type, except it has rows and columns set.
                (*this) = object_type(meta::type_wrapper<typename meta::glm_traits<T>::value_type>{});
                rows    = meta::glm_traits<T>::num_rows;
                columns = meta::glm_traits<T>::num_cols;
            }

            else {
                if constexpr      (std::is_integral_v<T>)       base_type = std::is_unsigned_v<T> ? base_type_t::UINT : base_type_t::INT;
                else if constexpr (std::is_floating_point_v<T>) base_type = base_type_t::FLOAT;
                else if constexpr (std::is_class_v<T>)          base_type = base_type_t::STRUCT;
                else static_assert(meta::always_false_v<T>, "Unknown C++ type for object type.");

                base_size = sizeof(typename meta::glm_traits<T>::value_type);


                if constexpr (std::is_class_v<T> && !meta::glm_traits<T>::is_glm) {
                    static_assert(is_decomposable_v<T>, "Cannot create object type for non-decomposable class.");

                    meta::create_pack::from_decomposable<T>::foreach([&] <typename M> {
                        struct_members.push_back(object_type(meta::type_wrapper<M>{}));
                    });
                }
            }
        }


        // Checks if this type can be used to initialize a vertex attribute of the given type in a GLSL shader.
        // Object types need not be exactly the same.
        // For example, a u8 can be used to initialize a u32 in the shader, since GLSL has no native u8 type.
        bool can_initialize_vertex_attribute(const object_type& other) const {
            if (base_type != other.base_type) return false;
            if (rows != other.rows) return false;
            if (columns != other.columns) return false;
            if (array_extents != other.array_extents) return false;

            // Vertex attributes may not be of struct type.
            if (!struct_members.empty()) return false;

            return true;
        }
    };
}