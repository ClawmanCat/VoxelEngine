#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/component/component_traits.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set_mixin.hpp>
#include <VoxelEngine/ecs/storage/component_storage/component_storage_mixin.hpp>
#include <VoxelEngine/ecs/storage/registry/component_storage_manager.hpp>
#include <VoxelEngine/ecs/storage/registry/system_manager.hpp>
#include <VoxelEngine/ecs/storage/registry/entity_lifetime_manager.hpp>
#include <VoxelEngine/ecs/storage/registry/query_index_manager.hpp>
#include <VoxelEngine/utility/meta/value.hpp>


namespace ve::ecs {
    template <typename T> concept registry_traits = requires {
        entity_traits<
            typename T::entity_traits
        >;

        component_traits<
            typename T::template component_traits<meta::placeholder_type>
        >;

        sparse_set_mixin_type<
            typename T::sparse_set_mixin,
            typename T::entity_traits
        >;

        component_storage_mixin_type<
            typename T::template component_storage_mixin<meta::placeholder_type>,
            typename T::entity_traits,
            typename T::template component_traits<meta::placeholder_type>
        >;
    };


    struct default_registry_traits {
        // Traits classes.
        using entity_traits = default_entity_traits<VE_DEFAULT_ENTITY_TYPE>;
        template <typename C> using component_traits = default_component_traits<C>;

        // Mixin classes.
        using sparse_set_mixin = no_sparse_set_mixin<entity_traits>;
        template <typename C> using component_storage_mixin = no_component_storage_mixin<entity_traits, component_traits<C>>;

        // Base classes.
        using entity_manager    = entity_lifetime_manager<entity_traits, sparse_set_mixin , sparse_set_mixin>;
        using component_manager = component_storage_manager<entity_traits, component_traits, sparse_set_mixin, component_storage_mixin>;
        using system_manager    = system_manager;
        using query_indexer     = query_index_manager<entity_traits>;
    };
}