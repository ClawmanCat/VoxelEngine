#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>


namespace ve::gfx {
    struct vertex_attribute {
        std::string_view name;

        enum base_type_t { INT, UINT, FLOAT } base_type;
        std::size_t base_size;
        std::size_t rows, columns;

        std::size_t offset, size;
    };

    template <typename T> constexpr static bool has_vertex_layout = requires { T::get_vertex_layout(); };


    template <typename T> constexpr vertex_attribute make_vertex_attribute(std::string_view name, std::size_t offset) {
        using base_type = typename meta::glm_traits<T>::value_type;
        static_assert(std::is_scalar_v<base_type>, "Unsupported vertex attribute type.");


        vertex_attribute result {
            .name      = name,
            .base_size = sizeof(base_type),
            .rows      = meta::glm_traits<T>::num_rows,
            .columns   = meta::glm_traits<T>::num_cols,
            .offset    = offset,
            .size      = sizeof(T)
        };

        if constexpr      (std::is_floating_point_v<base_type>) result.base_type = vertex_attribute::FLOAT;
        else if constexpr (std::is_signed_v<base_type>)         result.base_type = vertex_attribute::INT;
        else if constexpr (std::is_unsigned_v<base_type>)       result.base_type = vertex_attribute::UINT;

        return result;
    }


    #define ve_impl_vertex_layout_macro(R, D, E) \
    (ve::gfx::make_vertex_attribute<decltype(D::E)>(BOOST_PP_STRINGIZE(E), offsetof(D, E)))

    #define ve_vertex_layout(cls, ...)                      \
    constexpr static auto get_vertex_layout(void) {         \
        return std::array {                                 \
            BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(        \
                ve_impl_vertex_layout_macro,                \
                cls,                                        \
                BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)       \
            ))                                              \
        };                                                  \
    }


    namespace vertex_types {
        template <std::size_t N> struct color_vertex {
            vec<N, float> position;
            vec4ub color;

            ve_vertex_layout(color_vertex, position, color);
        };

        using color_vertex_2d = color_vertex<2>;
        using color_vertex_3d = color_vertex<3>;


        template <std::size_t N> struct texture_vertex {
            vec<N, float> position;

            vec2f uv;
            u8 texture_index;

            ve_vertex_layout(texture_vertex, position, uv, texture_index);
        };

        using texture_vertex_2d = texture_vertex<2>;
        using texture_vertex_3d = texture_vertex<3>;


        template <std::size_t N> struct material_vertex {
            vec<N, float> position;
            vec<N, float> normal;
            vec<N, float> tangent;

            // To reduce vertex size, assume all textures to be subtextures of the same texture.
            // The texture atlas provides a method to guarantee this is the case.
            vec2f uv_color;
            vec2f uv_normal;
            vec2f uv_material;
            u8 texture_index;


            ve_vertex_layout(
                material_vertex,
                position, normal, tangent,
                uv_color, uv_normal, uv_material,
                texture_index
            );
        };

        using material_vertex_2d = material_vertex<2>;
        using material_vertex_3d = material_vertex<3>;
    }
}