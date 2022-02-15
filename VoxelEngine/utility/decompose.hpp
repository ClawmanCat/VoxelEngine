#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/tuple_foreach.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/utility/traits/member_traits.hpp>
#include <VoxelEngine/utility/traits/is_std_array.hpp>

#include <boost/pfr.hpp>


namespace ve {
    #define ve_impl_decompose_macro(R, D, E) (&D::E)

    // Generates a method "get_members" in the current class,
    // which returns a tuple of pointers to the given member variables.
    #define ve_make_decomposable(cls, ...)                  \
    constexpr static auto get_members(void) {               \
        return std::tuple {                                 \
            BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(        \
                ve_impl_decompose_macro,                    \
                cls,                                        \
                BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)       \
            ))                                              \
        };                                                  \
    }


    #define ve_make_decomposable_with_base(cls, base, ...)  \
    constexpr static auto get_members(void) {               \
        constexpr auto my_members = std::tuple {            \
            BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_FOR_EACH(        \
                ve_impl_decompose_macro,                    \
                cls,                                        \
                BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)       \
            ))                                              \
        };                                                  \
                                                            \
        constexpr bool base_has_members =                   \
            [] <typename B = base> {                        \
                return requires { B::get_members(); };      \
            }();                                            \
                                                            \
        if constexpr (base_has_members) {                   \
            return std::tuple_cat(                          \
                base::get_members(),                        \
                my_members                                  \
            );                                              \
        } else {                                            \
            return my_members;                              \
        }                                                   \
    }


    template <typename T> constexpr static bool has_custom_decomposer_v = requires { T::get_members(); };


    // If this trait causes a compile error for your type add a nested typedef pfr_blacklist_tag to it.
    // e.g. struct my_type { using pfr_blacklist_tag = void; };
    // Note: std::array is not marked as decomposable since the very large sizes std::array will often have
    // will crash most if not all compilers.
    template <typename T> constexpr static bool is_decomposable_v =
        !requires { typename T::pfr_blacklist_tag; } &&
        !meta::is_std_array_v<T> &&
        (
            has_custom_decomposer_v<T> ||
            is_tuple_indexable_v<T>    ||
            std::is_aggregate_v<T>     ||
            std::is_scalar_v<T>
        );


    namespace detail {
        template <typename T> struct tuple_decomposer {
            template <std::size_t N> constexpr static auto& get(T& value) { return std::get<N>(value); }
            template <std::size_t N> constexpr static const auto& get(const T& value) { return std::get<N>(value); }

            constexpr static std::size_t size = std::tuple_size_v<T>;
            template <std::size_t N> using element_t = std::tuple_element_t<N, T>;
        };


        template <typename T> struct pfr_decomposer {
            template <std::size_t N> constexpr static auto& get(T& value) { return boost::pfr::get<N>(value); }
            template <std::size_t N> constexpr static const auto& get(const T& value) { return boost::pfr::get<N>(value); }

            constexpr static std::size_t size = boost::pfr::tuple_size_v<T>;
            template <std::size_t N> using element_t = boost::pfr::tuple_element_t<N, T>;
        };


        template <typename T> struct custom_decomposer {
            template <std::size_t N> constexpr static auto& get(T& value) {
                constexpr auto mem_ptr = std::get<N>(T::get_members());
                return value.*mem_ptr;
            }

            template <std::size_t N> constexpr static const auto& get(const T& value) {
                constexpr auto mem_ptr = std::get<N>(T::get_members());
                return value.*mem_ptr;
            }

            constexpr static std::size_t size = std::tuple_size_v<decltype(T::get_members())>;

            template <std::size_t N> using element_t = typename meta::member_traits<
                std::tuple_element_t<N, decltype(T::get_members())>
            >::member_type;
        };


        template <typename T> constexpr inline auto select_decomposer(void) {
            if constexpr (has_custom_decomposer_v<T>) {
                return custom_decomposer<T>{};
            }

            else if constexpr (is_tuple_indexable_v<T> && !meta::is_std_array_v<T>) {
                return tuple_decomposer<T>{};
            }

            // TODO: This will currently fail on aggregates with base classes, as PFR doesn't support them and triggers a static assert.
            // We need to find some way to check if the aggregate has a base class, but this seems to be a non-trivial task.
            else if constexpr ((std::is_aggregate_v<T> || std::is_scalar_v<T>) && !meta::is_std_array_v<T>) {
                return pfr_decomposer<T>{};
            }

            else static_assert(meta::always_false_v<T>, "Cannot decompose type.");
        }
    }


    // Generic decomposition interface for all types that are decomposable in some way,
    // be it through std::get or boost::pfr or ve_make_decomposable.
    template <typename T> requires is_decomposable_v<T>
    using decomposer_for = decltype(detail::select_decomposer<T>());
}