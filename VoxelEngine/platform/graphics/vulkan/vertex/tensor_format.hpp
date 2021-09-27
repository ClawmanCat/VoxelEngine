#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/vertex/vertex.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>
#include <VoxelEngine/utility/assert.hpp>

#include <vulkan/vulkan.hpp>


namespace ve::gfx::vulkan::detail {
    constexpr inline VkFormat vulkan_format_for_attribute(const vertex_attribute& attrib) {
        VE_DEBUG_ASSERT(attrib.columns == 1, "Vertex attributes of matrix type are not supported.");


        #define ve_impl_vkfmt_channel_seq (R)(G)(B)(A)
        #define ve_impl_vkfmt_num_channels (1)(2)(3)(4)
        #define ve_impl_vkfmt_size_seq (8)(16)(32)(64)
        #define ve_impl_vkfmt_type_seq ((FLOAT, SFLOAT))((INT, SINT))((UINT, UINT))


        // Generates the channels + size part (e.g. R8G8B8 or R32G32B32A32) of the format.
        #define ve_impl_vkfmt_fmt_macro(Z, N, D) \
        (BOOST_PP_CAT(BOOST_PP_SEQ_ELEM(N, ve_impl_vkfmt_channel_seq), D))

        #define ve_impl_vkfmt_fmt(count, size, type)                                    \
        BOOST_PP_SEQ_CAT(                                                               \
            (VK_FORMAT_)                                                                \
            BOOST_PP_REPEAT(                                                            \
                count,                                                                  \
                ve_impl_vkfmt_fmt_macro,                                                \
                size                                                                    \
            )                                                                           \
            (_)                                                                         \
            (type)                                                                      \
        )


        // Some formats like VK_FORMAT_R8_SFLOAT don't actually exists, so filter those out.
        // Note: lambda needs to be templated on the enum to prevent the requires expression from evaluating early,
        // and just causing a compilation error.
        #define ve_impl_vkfmt_check_exists(fmt)                                         \
        [] <typename E1 = VkFormat> {                                                   \
            return requires { E1::fmt; };                                               \
        }()

        // Prevent directly (i.e. not in a non-templated context) referencing the type if it doesn't exist,
        // for the same reasons mentioned above.
        #define ve_impl_vkfmt_get_format(fmt)                                           \
        [] <typename E2 = VkFormat> {                                                   \
            if constexpr (ve_impl_vkfmt_check_exists(fmt)) {                            \
                return E2::fmt;                                                         \
            } else return VK_FORMAT_UNDEFINED;                                          \
        }()


        // If the given format exists, create a check for if the attribute matches that format,
        // and return the appropriate VkFormat if this is the case.
        #define ve_impl_vkfmt_case(num_channels, size, ve_type, vk_type)                \
        if constexpr (                                                                  \
            ve_impl_vkfmt_check_exists(ve_impl_vkfmt_fmt(num_channels, size, vk_type))  \
        ) {                                                                             \
            if (                                                                        \
                attrib.base_type == vertex_attribute::ve_type &&                        \
                attrib.base_size == size &&                                             \
                attrib.rows      == num_channels                                        \
            ) {                                                                         \
                return ve_impl_vkfmt_get_format(                                        \
                    ve_impl_vkfmt_fmt(num_channels, size, vk_type)                      \
                );                                                                      \
            }                                                                           \
        }

        #define ve_impl_vkfmt_case_wrapper(R, Seq)                                      \
        ve_impl_vkfmt_case(                                                             \
            BOOST_PP_SEQ_ELEM(0, Seq),                                                  \
            BOOST_PP_SEQ_ELEM(1, Seq),                                                  \
            BOOST_PP_TUPLE_ELEM(0, BOOST_PP_SEQ_ELEM(2, Seq)),                          \
            BOOST_PP_TUPLE_ELEM(1, BOOST_PP_SEQ_ELEM(2, Seq))                           \
        )


        // Invoke ve_impl_vkfmt_case for every combination of number of channels, size and scalar type.
        BOOST_PP_SEQ_FOR_EACH_PRODUCT(
            ve_impl_vkfmt_case_wrapper,
            (ve_impl_vkfmt_num_channels)
            (ve_impl_vkfmt_size_seq)
            (ve_impl_vkfmt_type_seq)
        )


        VE_UNREACHABLE;
    }
}