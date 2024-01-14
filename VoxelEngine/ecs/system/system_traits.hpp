#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/meta/pack/pack_operations.hpp>
#include <VoxelEngine/utility/meta/common_concepts.hpp>
#include <VoxelEngine/ecs/view/query/query_building_blocks.hpp>


namespace ve::ecs {
    /** System component matcher must provide either a query or lists of included/excluded/optional components. */
    template <typename SCM> concept system_component_matcher = requires {
        requires (
            requires {
                typename SCM::query;
            } ||
            requires {
                typename SCM::included_components;
                typename SCM::excluded_components;
                typename SCM::optional_components;
            }
        );
    };


    /** Specification for how execution of the system is sequenced internally and with regards to other systems. */
    template <typename SOS> concept system_ordering_specification = requires {
        // Tags to identify this system in the run_before and run_after lists of other systems.
        meta::type_pack<typename SOS::sequencing_tags>;

        // List of tags for which, if some other system has it, this system will be sequenced before/after/not during that system.
        meta::type_pack<typename SOS::run_before>;
        meta::type_pack<typename SOS::run_after>;
        meta::type_pack<typename SOS::run_not_during>;

        // List of component types which are read by this system.
        // This should include any components accessed in the view that is used to invoke the system.
        meta::type_pack<typename SOS::read_components>;
        // List of component types which are written to by this system.
        meta::type_pack<typename SOS::write_components>;
        // List of component types which are added/removed by this system.
        meta::type_pack<typename SOS::add_remove_components>;

        // Set to true if this system modifies existing entity IDs in some way.
        { SOS::modifies_entities    } -> meta::const_member_type<bool>;
        // Set to true if this system adds or removes entities from the ECS.
        { SOS::add_remove_entities  } -> meta::const_member_type<bool>;
        // Set to true if this system requires that no other systems be run at the same time as this one.
        { SOS::requires_exclusive   } -> meta::const_member_type<bool>;
        // Set to true if this system can only be invoked on the main thread.
        { SOS::requires_main_thread } -> meta::const_member_type<bool>;
        // Set to true if it is allowed to split the set of entities matched by this system into groups
        // and invoke the system concurrently on multiple threads with a view of each of these groups.
        { SOS::parallelizable       } -> meta::const_member_type<bool>;
    };


    /** A system must specify which entities/components it accesses and how it should be sequenced with regards to other systems. */
    template <typename Traits> concept system_traits = requires {
        system_component_matcher<typename Traits::viewed_components>;
        system_ordering_specification<typename Traits::ordering_specification>;
    };


    /**
     * Utility to generate a new system traits class by changing an existing one.
     * @tparam Traits The system traits to transform.
     */
    template <system_traits Traits> class transform_system_traits {
    private:
        template <typename Q> struct scm_query_builder {
            using query = Q;
        };

        template <typename I, typename E, typename O> struct scm_include_list_builder {
            using included_components = I;
            using excluded_components = E;
            using optional_components = O;
        };


        template <
            typename ST, typename RB, typename RA, typename RND, typename RC, typename WC, typename ARC,
            bool ME, bool ARE, bool RE, bool RMT, bool P
        > struct ordering_specification_builder {
            using sequencing_tags       = ST;
            using run_before            = RB;
            using run_after             = RA;
            using run_not_during        = RND;
            using read_components       = RC;
            using write_components      = WC;
            using add_remove_components = ARC;

            constexpr static inline bool modifies_entities    = ME;
            constexpr static inline bool add_remove_entities  = ARE;
            constexpr static inline bool requires_exclusive   = RE;
            constexpr static inline bool requires_main_thread = RMT;
            constexpr static inline bool parallelizable       = P;
        };

        template <typename T, std::size_t N> using edit_ordering_specification_at = ordering_specification_builder<
            std::conditional_t<(N == 0), typename Traits::ordering_specification::sequencing_tags,       T>,
            std::conditional_t<(N == 1), typename Traits::ordering_specification::run_before,            T>,
            std::conditional_t<(N == 2), typename Traits::ordering_specification::run_after,             T>,
            std::conditional_t<(N == 3), typename Traits::ordering_specification::run_not_during,        T>,
            std::conditional_t<(N == 4), typename Traits::ordering_specification::read_components,       T>,
            std::conditional_t<(N == 5), typename Traits::ordering_specification::write_components,      T>,
            std::conditional_t<(N == 6), typename Traits::ordering_specification::add_remove_components, T>,
            (N == 7)  ? T::value : Traits::ordering_specification::modifies_entities,
            (N == 8)  ? T::value : Traits::ordering_specification::add_remove_entities,
            (N == 9)  ? T::value : Traits::ordering_specification::requires_exclusive,
            (N == 10) ? T::value : Traits::ordering_specification::requires_main_thread,
            (N == 11) ? T::value : Traits::ordering_specification::requires_parallelizable
        >;


        template <system_component_matcher SCM, system_ordering_specification SOS> struct traits_builder {
            using viewed_components = SCM;
            using ordering_specification = SOS;
        };

    public:
        template <query::ecs_query Q>
        using with_query = traits_builder<scm_query_builder<Q>, typename Traits::ordering_specification>;

        template <meta::type_pack Include, meta::type_pack Exclude = meta::pack<>, meta::type_pack Optional = meta::pack<>>
        using with_components = traits_builder<scm_include_list_builder<Include, Exclude, Optional>, typename Traits::ordering_specification>;


        template <typename... Components> using add_include_components = with_components<
            typename Traits::viewed_components::included_components::template append<Components...>,
            typename Traits::viewed_components::excluded_components,
            typename Traits::viewed_components::optional_components
        >;

        template <typename... Components> using add_exclude_components = with_components<
            typename Traits::viewed_components::included_components,
            typename Traits::viewed_components::excluded_components::template append<Components...>,
            typename Traits::viewed_components::optional_components
        >;

        template <typename... Components> using add_optional_components = with_components<
            typename Traits::viewed_components::included_components,
            typename Traits::viewed_components::excluded_components,
            typename Traits::viewed_components::optional_components::template append<Components...>
        >;

        template <typename... Components> using remove_include_components = with_components<
            meta::pack_difference<typename Traits::viewed_components::included_components, meta::pack<Components...>>,
            typename Traits::viewed_components::excluded_components,
            typename Traits::viewed_components::optional_components
        >;

        template <typename... Components> using remove_exclude_components = with_components<
            typename Traits::viewed_components::included_components,
            meta::pack_difference<typename Traits::viewed_components::excluded_components, meta::pack<Components...>>,
            typename Traits::viewed_components::optional_components
        >;

        template <typename... Components> using remove_optional_components = with_components<
            typename Traits::viewed_components::included_components,
            typename Traits::viewed_components::excluded_components,
            meta::pack_difference<typename Traits::viewed_components::optional_components, meta::pack<Components...>>
        >;


        #define VE_IMPL_MODIFY_OSPEC_PACK_TRAIT(Name, Element, N)                                           \
        template <typename... Ts> using with_##Name = traits_builder<                                       \
            typename Traits::viewed_components,                                                             \
            edit_ordering_specification_at<meta::pack<Ts...>, N>                                            \
        >;                                                                                                  \
                                                                                                            \
        template <typename... Ts> using add_##Name = with_##Name<                                           \
            typename Traits::ordering_specification::Element::template append<Ts...>                        \
        >;                                                                                                  \
                                                                                                            \
        template <typename... Ts> using remove_##Name = with_##Name<                                        \
            meta::pack_difference<typename Traits::ordering_specification::Element, meta::pack<Ts...>>      \
        >;


        VE_IMPL_MODIFY_OSPEC_PACK_TRAIT(sequencing_tags,       sequencing_tags,       0);
        VE_IMPL_MODIFY_OSPEC_PACK_TRAIT(run_before_tags,       run_before,            1);
        VE_IMPL_MODIFY_OSPEC_PACK_TRAIT(run_after_tags,        run_after,             2);
        VE_IMPL_MODIFY_OSPEC_PACK_TRAIT(run_not_during_tags,   run_not_during,        3);
        VE_IMPL_MODIFY_OSPEC_PACK_TRAIT(read_components,       read_components,       4);
        VE_IMPL_MODIFY_OSPEC_PACK_TRAIT(write_components,      write_components,      5);
        VE_IMPL_MODIFY_OSPEC_PACK_TRAIT(add_remove_components, add_remove_components, 6);


        template <bool Active> using set_modifies_entities    = traits_builder<typename Traits::viewed_components, edit_ordering_specification_at<meta::value<Active>, 7>>;
        template <bool Active> using set_add_remove_entities  = traits_builder<typename Traits::viewed_components, edit_ordering_specification_at<meta::value<Active>, 8>>;
        template <bool Active> using set_requires_exclusive   = traits_builder<typename Traits::viewed_components, edit_ordering_specification_at<meta::value<Active>, 9>>;
        template <bool Active> using set_requires_main_thread = traits_builder<typename Traits::viewed_components, edit_ordering_specification_at<meta::value<Active>, 10>>;
        template <bool Active> using set_parallelizable       = traits_builder<typename Traits::viewed_components, edit_ordering_specification_at<meta::value<Active>, 11>>;
    };
}