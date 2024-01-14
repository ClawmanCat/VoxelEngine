#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>
#include <VoxelEngine/utility/meta/pack/create_pack.hpp>


namespace ve::ecs::detail {
    namespace projections {
        template <typename V> using entity_type     = typename V::entity_type;
        template <typename V> using entity_traits   = typename V::entity_traits;
        template <typename V> using entity_utils    = typename V::entity_utils;
        template <typename V> using component_types = typename V::component_types;
    }


    /** Projects the component types within an [entity, components...] tuple. */
    template <template <typename> typename Projection>
    struct project_value_type_components {
        template <typename Tuple, typename P = meta::create_pack::from_template<Tuple>>
            using type = typename meta::create_pack::from_many<
                meta::pack<typename P::head>,
                typename P::template pop_front<1>::template map<Projection>
            >::template to<std::tuple>;
    };


    /** Returns the common type of all views after applying the given projection. */
    template <meta::type_pack Views, template <typename> typename Projection>
    using common_view_projection = typename Views
        ::template map<Projection>
        ::template to<std::common_type_t>;
}