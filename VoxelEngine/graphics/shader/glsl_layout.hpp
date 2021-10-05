#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/bit.hpp>
#include <VoxelEngine/utility/decompose.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/utility/traits/is_std_array.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve::gfx {
    enum class glsl_layout { STD140, STD430 };


    namespace detail {
        template <glsl_layout Layout, typename T>
        constexpr std::size_t get_glsl_alignment(void) {
            using scalars_alignof_4 = meta::pack<bool, i32, u32, f32>;
            using scalars_alignof_8 = meta::pack<f64>;
            using glm_traits        = meta::glm_traits<T>;


            // Align scalar types either to 4 or 8 bytes.
            if constexpr (scalars_alignof_4::template contains<T>) {
                return 4;
            }

            else if constexpr (scalars_alignof_8::template contains<T>) {
                return 8;
            }


            // Align vector types to either two or four times the alignment of the contained scalar.
            else if constexpr (glm_traits::is_vector) {
                return get_glsl_alignment<Layout, typename glm_traits::value_type>() * (glm_traits::num_rows <= 2 ? 2 : 4);
            }


            // Align matrix types as if they consisted of 4 vec4s.
            // Note that GLSL only supports matrices of f32 type.
            else if constexpr (glm_traits::is_matrix && std::is_same_v<typename glm_traits::value_type, f32>) {
                return get_glsl_alignment<Layout, vec4f>();
            }


            // STD140: Align the array and each element to max(alignof(element), 16)
            // (Yes, each element individually. Yes, this wastes massive amounts of space.)
            // STD430: Align the array and each element to alignof(element)
            else if constexpr (std::is_array_v<T> || meta::is_std_array_v<T>) {
                using vt = std::remove_cvref_t<decltype(std::declval<T>()[0])>;

                return (Layout == glsl_layout::STD140)
                    ? std::max(get_glsl_alignment<Layout, vt>(), (std::size_t) 16)
                    : get_glsl_alignment<Layout, vt>();
            }


            // STD140: Align the struct to max(alignof(most_aligned_element), 16)
            // STD430: Align the struct to alignof(most_aligned_element).
            else if constexpr (std::is_class_v<T> && is_decomposable_v<T>) {
                using member_types = meta::create_pack::from_decomposable<T>;

                std::size_t alignment = (Layout == glsl_layout::STD140 ? 16 : 0);
                member_types::foreach_indexed([&] <typename M, std::size_t I> {
                    alignment = std::max(alignment, get_glsl_alignment<Layout, M>());
                });

                return alignment;
            }


            else static_assert(meta::always_false_v<T>, "Cannot calculate GLSL alignment for type.");
        }


        // Appends the given object to the given array, adding padding where needed to meet the given alignment requirements.
        // Note that unlike CPU-side objects, STD140/430 objects don't receive any end padding. E.g.:
        // struct { vec3 a; float b; } will have a size of 16, even though vec3s have an alignment of 16 themselves.
        // This means it is often not possible to create a valid object representation CPU-side, and the only way to store
        // the object is as an array of bytes.
        template <glsl_layout Layout, typename T>
        inline void push_glsl_layout(const T& value, std::vector<u8>& result, bool top_level = true) {
            using glm_traits = meta::glm_traits<T>;


            auto push_alignment = [&] (std::size_t alignment) {
                std::size_t start_address = next_aligned_address(result.size(), alignment);
                result.resize(start_address, (u8) 0x00);
            };

            auto push_data = [&] (const auto& object, std::size_t alignment) {
                std::size_t start_address = next_aligned_address(result.size(), alignment);

                result.resize(start_address + sizeof(object), (u8) 0x00);
                memcpy(&result[start_address], &object, sizeof(object));
            };


            // Align scalar types either to 4 or 8 bytes.
            if constexpr (std::is_scalar_v<T>) {
                push_data(value, get_glsl_alignment<Layout, T>());
            }


            // Align vector types to either two or four times the alignment of the contained scalar.
            else if constexpr (glm_traits::is_vector) {
                push_alignment(get_glsl_alignment<Layout, T>());
                for (std::size_t i = 0; i < glm_traits::num_rows; ++i) push_glsl_layout<Layout>(value[i], result, false);
            }


            // Align matrix types as if they consisted of 4 vec4s.
            // Note that GLSL only supports matrices of f32 type.
            else if constexpr (glm_traits::is_matrix && std::is_same_v<typename glm_traits::value_type, f32>) {
                push_alignment(get_glsl_alignment<Layout, T>());
                for (std::size_t i = 0; i < glm_traits::num_cols; ++i) push_glsl_layout<Layout>(value[i], result, false);
            }


            // STD140: Align the array and each element to max(alignof(element), 16)
            // (Yes, each element individually. Yes, this wastes massive amounts of space.)
            // STD430: Align the array and each element to alignof(element)
            else if constexpr (std::is_array_v<T> || meta::is_std_array_v<T>) {
                for (const auto& elem : value) {
                    push_alignment(get_glsl_alignment<Layout, T>());
                    push_glsl_layout<Layout>(elem, result, false);
                }

                // Unlike other types, arrays do get end padding.
                push_alignment(get_glsl_alignment<Layout, T>());
            }


            // STD140: Align the struct to max(alignof(most_aligned_element), 16)
            // STD430: Align the struct to alignof(most_aligned_element).
            else if constexpr (std::is_class_v<T> && is_decomposable_v<T>) {
                using member_types = meta::create_pack::from_decomposable<T>;

                push_alignment(get_glsl_alignment<Layout, T>());
                member_types::foreach_indexed([&] <typename M, std::size_t I> {
                    push_glsl_layout<Layout>(decomposer_for<T>::template get<I>(value), result, false);
                });

                // Unlike other types, structs do get end padding, except when the struct is the UBO / SSBO itself.
                if (!top_level) push_alignment(get_glsl_alignment<Layout, T>());
            }


            else static_assert(meta::always_false_v<T>, "Cannot generate GLSL layout for type.");
        }
    }


    template <glsl_layout Layout, typename T>
    inline std::vector<u8> to_glsl_layout(const T& value) {
        std::vector<u8> result;
        detail::push_glsl_layout<Layout>(value, result);
        return result;
    }

    template <typename T> inline std::vector<u8> to_std140(const T& value) {
        return to_glsl_layout<glsl_layout::STD140>(value);
    }

    template <typename T> inline std::vector<u8> to_std430(const T& value) {
        return to_glsl_layout<glsl_layout::STD430>(value);
    }


    // Clears the existing storage and inserts the GLSL layout.
    // Will prevent heap allocation calls if the storage is already large enough.
    template <glsl_layout Layout, typename T>
    inline void store_glsl_layout_into(const T& value, std::vector<u8>& result) {
        result.clear();
        detail::push_glsl_layout<Layout>(value, result);
    }

    template <typename T> inline void store_std140_into(const T& value, std::vector<u8>& result) {
        store_glsl_layout_into<glsl_layout::STD140>(value, result);
    }

    template <typename T> inline void store_std430_into(const T& value, std::vector<u8>& result) {
        store_glsl_layout_into<glsl_layout::STD430>(value, result);
    }
}