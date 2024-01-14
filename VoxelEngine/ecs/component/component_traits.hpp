#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/common_concepts.hpp>


namespace ve::ecs {
    /** Component types must be at least movable and destructible. */
    template <typename T> concept component_type =
        std::is_move_constructible_v<T> &&
        std::is_destructible_v<T>;

    /** To elude storage, the component must be empty and destroying the component must be non-observable (trivial). */
    template <typename T> concept storage_eludable_component_type =
        std::is_empty_v<T> &&
        std::is_trivially_destructible_v<T>;


    /**
     * Concept indicating a class can be used as component traits for an ECS container.
     * See @ref default_component_traits for an example implementation of an component traits class.
     */
    template <typename T> concept component_traits = requires {
        { T::reference_stability } -> meta::const_member_type<bool>;
        { T::elude_storage       } -> meta::const_member_type<bool>;
        { T::page_size           } -> meta::const_member_type<std::size_t>;
        // Component type must be a valid component.
        component_type<typename T::type>;
        // If storage elusion is enabled, the component type must be storage eludable.
        (!T::elude_storage || storage_eludable_component_type<typename T::type>);
        // Page size must be a power of two.
        std::popcount(T::page_size) == 1;
    };


    /** Returns true if two entity traits classes represent the same traits, even if they are different classes. */
    template <component_traits T1, component_traits T2> constexpr inline bool component_traits_equivalent = (
        std::is_same_v<typename T1::type, typename T2::type> &&
        T1::reference_stability == T2::reference_stability   &&
        T1::elude_storage       == T2::elude_storage         &&
        T1::page_size           == T2::page_size
    );


    /** Default component_traits class used if no other component traits are provided to an ECS container. */
    template <component_type Component> struct default_component_traits {
        /** The type of component these traits are associated with. */
        using type = Component;

        /**
         * If set to true, components using the given traits will have a constant address during their lifetime.
         * This implicitly sets index_stability to true on the associated entity traits.
         */
        constexpr static inline bool reference_stability = false;

        /**
         * If set to true, components will not actually be stored in the ECS. Instead, a reference to a dummy component is produced whenever the component is requested.
         * This is only allowed for empty types which can be moved and deleted non-observably (trivially).
         */
        constexpr static inline bool elude_storage = storage_eludable_component_type<Component>;

        /**
         * Size for pages in component pools in number of components. Must be a power of two.
         * Usage of paging reduces memory usage from large unoccupied component ranges.
         */
        constexpr static inline std::size_t page_size = 1024ull;
    };


    /**
     * Utility to generate a new component traits class by changing an existing one.
     * @tparam Traits The entity component to transform.
     */
    template <component_traits Traits> struct transform_component_traits {
        /** Transformed traits class with new settings. */
        template <component_type CT, bool rs, bool es, std::size_t ps> struct traits_builder {
            using type = CT;

            constexpr static inline bool reference_stability = rs;
            constexpr static inline bool elude_storage       = es;
            constexpr static inline std::size_t page_size    = ps;
        };


        /**
         * Changes the component type of the current traits.
         * Note: this will also set elude_storage to false if it is not valid for the new component type.
         */
        template <component_type T> using with_component_type = traits_builder<
            T,
            Traits::reference_stability,
            storage_eludable_component_type<T> ? Traits::elude_storage : false,
            Traits::page_size
        >;

        template <bool ReferenceStability> using with_reference_stability = traits_builder<
            typename Traits::type,
            ReferenceStability,
            Traits::elude_storage,
            Traits::page_size
        >;

        template <bool EludeStorage> using with_storage_elusion = traits_builder<
            typename Traits::type,
            Traits::reference_stability,
            EludeStorage,
            Traits::page_size
        >;

        template <std::size_t PageSize> using with_page_size = traits_builder<
            typename Traits::type,
            Traits::reference_stability,
            Traits::elude_storage,
            PageSize
        >;
    };


    /** Utility functions associated with a set of component traits. */
    template <component_traits Traits> struct component_utils {
        using traits_type    = Traits;
        using component_type = typename Traits::type;

        // Note: currently none.
    };
}