#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/function_traits.hpp>
#include <VoxelEngine/utility/meta/pack/create_pack.hpp>

#include <boost/preprocessor.hpp>

#include <tuple>
#include <functional>


#define VE_IMPL_DECOMPOSE_INTO_FIELD(R, D, E) ( &D::E )

#define VE_DECOMPOSE_INTO(Class, ...)                       \
[[nodiscard]] consteval static auto decompose(void) {       \
    return std::tuple {                                     \
        BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(            \
            VE_IMPL_DECOMPOSE_INTO_FIELD,                   \
            Class,                                          \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)           \
        ))                                                  \
    };                                                      \
}


namespace ve::detail {
    /** Decomposer for classes that fulfil the @ref custom_decomposable concept (See below).  */
    template <typename T> struct custom_decomposer_adaptor {
        constexpr static inline bool is_decomposable = true;
        constexpr static inline std::size_t size = std::tuple_size_v<decltype(T::decompose())>;


        constexpr static auto reference_as_tuple(const T& value) {
            return std::apply(
                [&] (const auto&... accessors) {
                    return std::forward_as_tuple(std::invoke(accessors, value)...);
                },
                T::decompose()
            );
        }


        constexpr static auto reference_as_tuple(T& value) {
            return std::apply(
                [&] (const auto&... accessors) {
                    return std::forward_as_tuple(std::invoke(accessors, value)...);
                },
                T::decompose()
            );
        }


        constexpr static auto copy_as_tuple(const T& value) {
            return std::apply(
                [&] <typename... Accessors> (const Accessors&... accessors) {
                    return std::tuple<
                        typename meta::function_traits<Accessors>::return_type...
                    >(std::invoke(accessors, value)...);
                },
                T::decompose()
            );
        }


        using decomposed_type = T;
        using member_types    = ve::meta::create_pack::from_template<decltype(copy_as_tuple(std::declval<T>()))>;
    };


    /** A class is custom_decomposable if it has a static method decompose() which returns a tuple of member variable pointers for that class. */
    template <typename T> concept custom_decomposable = requires {
        { T::decompose() } -> meta::template_instantiation_of<std::tuple>;

        meta::create_pack::from_template<decltype(T::decompose())>::all([] <typename Accessor> {
            return (requires (T obj, Accessor access) {
                { std::invoke(access, obj) } -> meta::member_variable_pointer;

                std::is_same_v<
                    typename meta::function_traits<decltype(std::invoke(access, obj))>::owning_class,
                    T
                >;
            });
        });
    };
}