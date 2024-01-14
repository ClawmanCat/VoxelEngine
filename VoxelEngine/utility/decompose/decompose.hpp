#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/decompose/custom_decomposer.hpp>
#include <VoxelEngine/utility/decompose/generated_decomposers.hpp>

#include <concepts>


namespace ve {
    /**
     * Concept for decomposer classes.
     * Decomposer classes are classes that can decompose another class (D::decomposed_type) into a tuple of its members.
     */
    template <typename D> concept decomposer = requires (
        typename D::decomposed_type obj,
        decltype(D::reference_as_tuple(obj)) mut_reference_tuple,
        decltype(D::reference_as_tuple(std::as_const(obj))) const_reference_tuple,
        decltype(D::copy_as_tuple(std::as_const(obj))) copy_tuple
    ) {
        D::is_decomposable;

        { D::size } -> std::convertible_to<std::size_t>;
        { typename D::member_types {} } -> meta::type_pack;

        meta::is_template_v<std::tuple, decltype(mut_reference_tuple)>;
        meta::is_template_v<std::tuple, decltype(const_reference_tuple)>;
        meta::is_template_v<std::tuple, decltype(copy_tuple)>;

        meta::create_pack::from_template<decltype(mut_reference_tuple)>::all([] <typename MR> {
            return std::is_lvalue_reference_v<MR>; // Note: could still be const if the actual data member is const.
        });

        meta::create_pack::from_template<decltype(const_reference_tuple)>::all([] <typename CR> {
            return std::is_lvalue_reference_v<CR> && std::is_const_v<std::remove_reference_t<CR>>;
        });

        // Note: no checks on copy_tuple, members could still be references or const if the actual data member has these qualifiers.
    };


    /** Checks that D is a valid decomposer (See @ref decomposer) for the type T. */
    template <typename D, typename T> concept type_decomposer = decomposer<D> && std::is_same_v<typename D::decomposed_type, T>;


    template <typename T> concept binding_decomposable = detail::binding_decomposable<T>;
    template <typename T> concept custom_decomposable  = detail::custom_decomposable<T>;


    /** Returns the decomposer for the given type. This template may be further specialized for other decomposer implementations. */
    template <typename> struct decomposer_for {};

    template <binding_decomposable T> struct decomposer_for<T> { using type = detail::n_element_decomposer<T>;      };
    template <custom_decomposable  T> struct decomposer_for<T> { using type = detail::custom_decomposer_adaptor<T>; };


    /** True if a type is decomposable, i.e. decomposer_for is specialized and valid for the given type. */
    template <typename T> concept decomposable = type_decomposer<typename decomposer_for<T>::type, T>;

    /** Returns the decomposer for the given type. This template may be further specialized through decomposer_for for other decomposer implementations. */
    template <decomposable T> using decomposer_for_t = typename decomposer_for<T>::type;


    /** Returns a tuple of references to each member of 'value'. */
    template <decomposable T> constexpr inline auto decompose_as_references(T& value) {
        return decomposer_for_t<T>::reference_as_tuple(value);
    }

    /** Returns a tuple of const references to each member of 'value'. */
    template <decomposable T> constexpr inline auto decompose_as_const_references(const T& value) {
        return decomposer_for_t<T>::reference_as_tuple(value);
    }

    /** Returns a tuple of copies of each member of 'value'. */
    template <decomposable T> constexpr inline auto decompose_as_copy(const T& value) {
        return decomposer_for_t<T>::copy_as_tuple(value);
    }
}