#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/ecs/system/system_traits.hpp>
#include <VoxelEngine/ecs/view/query/query_building_blocks.hpp>
#include <VoxelEngine/ecs/view/query/query_utils.hpp>


namespace ve::ecs::system_traits_templates {
    namespace detail {
        template <typename Q> struct query_scm { using query = Q; };

        template <typename I, typename E, typename O> struct type_list_scm {
            using included_components = I;
            using excluded_components = E;
            using optional_components = O;
        };
    }


    /** System traits template for a system that only reads from the ECS and does not modify it. */
    template <system_component_matcher SCM> struct read_only_system_tmpl {
        using viewed_components = SCM;

        struct ordering_specification {
            using sequencing_tags = meta::pack<>;
            using run_before      = meta::pack<>;
            using run_after       = meta::pack<>;
            using run_not_during  = meta::pack<>;

            using read_components       = meta::pack<>;
            using write_components      = meta::pack<>;
            using add_remove_components = meta::pack<>;

            constexpr static inline bool modifies_entities    = false;
            constexpr static inline bool add_remove_entities  = false;
            constexpr static inline bool requires_exclusive   = false;
            constexpr static inline bool requires_main_thread = false;
            constexpr static inline bool parallelizable       = true;
        };
    };

    template <query::ecs_query Query>
    using read_only_system_for_query = read_only_system_tmpl<detail::query_scm<Query>>;

    template <meta::type_pack Include, meta::type_pack Exclude = meta::pack<>, meta::type_pack Optional = meta::pack<>>
    using read_only_system_for_types = read_only_system_tmpl<detail::type_list_scm<Include, Exclude, Optional>>;


    /** System traits template for a system that both reads from and writes to the ECS. */
    template <system_component_matcher SCM, meta::type_pack W> struct read_write_system_tmpl {
        using viewed_components = SCM;

        struct ordering_specification {
            using sequencing_tags = meta::pack<>;
            using run_before      = meta::pack<>;
            using run_after       = meta::pack<>;
            using run_not_during  = meta::pack<>;

            using read_components       = meta::pack<>;
            using write_components      = W;
            using add_remove_components = meta::pack<>;

            constexpr static inline bool modifies_entities    = false;
            constexpr static inline bool add_remove_entities  = false;
            constexpr static inline bool requires_exclusive   = false;
            constexpr static inline bool requires_main_thread = false;
            constexpr static inline bool parallelizable       = true;
        };
    };

    template <query::ecs_query Query, meta::type_pack Write = meta::create_pack::from_many<query::detail::query_included_components<Query>, query::detail::query_optional_components<Query>>>
    using read_write_system_for_query = read_only_system_tmpl<detail::query_scm<Query>>;

    template <meta::type_pack Include, meta::type_pack Exclude = meta::pack<>, meta::type_pack Optional = meta::pack<>>
    using read_write_system_for_types = read_only_system_tmpl<detail::type_list_scm<Include, Exclude, Optional>>;
}