#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/entity_traits.hpp>
#include <VoxelEngine/ecs/component/component_traits.hpp>
#include <VoxelEngine/ecs/storage/sparse_set/sparse_set.hpp>
#include <VoxelEngine/utility/meta/common_concepts.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


namespace ve::ecs {
    /**
     * Concept for classes that can be used as an ecs_view.
     * An ecs_view is an iterable object that iterates values of std::tuple<Entity, P<Components>...> where P is some projection applied to each component (e.g. add_reference or add_pointer).
     * It is allowed for different components in the view to have different projections.
     * For example a view could iterate TypeA and optionally TypeB with a value_type of std::tuple<Entity, TypeA&, TypeB*>.
     */
    template <typename View> concept ecs_view = requires (View v, const View cv, typename View::entity_type entt) {
        // Required typedefs
        typename View::entity_type;
        typename View::entity_traits;
        typename View::entity_utils;
        typename View::component_types;
        typename View::value_type;
        typename View::const_value_type;
        typename View::reference;
        typename View::const_reference;
        typename View::pointer;
        typename View::const_pointer;
        typename View::size_type;
        typename View::difference_type;
        typename View::iterator;
        typename View::const_iterator;
        typename View::reverse_iterator;
        typename View::const_reverse_iterator;

        
        // Typedef constraints
        entity_type<typename View::entity_type>;
        entity_traits<typename View::entity_traits>;
        std::is_same_v<typename View::entity_type, typename View::entity_traits::type>;
        std::is_same_v<typename View::entity_utils, entity_utils<typename View::entity_traits>>;
        meta::type_pack<typename View::component_types>;
        View::component_types::all([] <typename C> { return component_type<C>; });
        meta::is_template_v<std::tuple, typename View::value_type>;
        meta::is_template_v<std::tuple, typename View::const_value_type>;
        meta::is_template_v<std::tuple, typename View::reference>;
        meta::is_template_v<std::tuple, typename View::const_reference>;
        meta::is_template_v<std::tuple, std::remove_cvref_t<decltype(*std::declval<typename View::pointer>())>>;
        meta::is_template_v<std::tuple, std::remove_cvref_t<decltype(*std::declval<typename View::const_pointer>())>>;
        std::is_same_v<typename View::size_type, std::size_t>;
        std::is_same_v<typename View::difference_type, std::ptrdiff_t>;
        std::is_same_v<typename View::entity_type, std::tuple_element_t<0, typename View::value_type>>;
        std::bidirectional_iterator<typename View::iterator>;
        std::bidirectional_iterator<typename View::const_iterator>;
        std::bidirectional_iterator<typename View::reverse_iterator>;
        std::bidirectional_iterator<typename View::const_reverse_iterator>;


        // Required methods.
        { cv.contains(entt) } -> std::same_as<bool>;
        { cv[entt]          } -> std::same_as<typename View::const_reference>;
        { v[entt]           } -> std::same_as<typename View::reference>;
        { cv.begin()        } -> std::same_as<typename View::const_iterator>;
        { cv.cbegin()       } -> std::same_as<typename View::const_iterator>;
        { cv.rbegin()       } -> std::same_as<typename View::const_reverse_iterator>;
        { cv.crbegin()      } -> std::same_as<typename View::const_reverse_iterator>;
        { v.begin()         } -> std::same_as<typename View::iterator>;
        { v.rbegin()        } -> std::same_as<typename View::reverse_iterator>;
        { cv.end()          } -> std::same_as<typename View::const_iterator>;
        { cv.cend()         } -> std::same_as<typename View::const_iterator>;
        { cv.rend()         } -> std::same_as<typename View::const_reverse_iterator>;
        { cv.crend()        } -> std::same_as<typename View::const_reverse_iterator>;
        { v.end()           } -> std::same_as<typename View::iterator>;
        { v.rend()          } -> std::same_as<typename View::reverse_iterator>;
        

        // View iterator requirements.
        meta::pack<
            typename View::iterator,
            typename View::const_iterator,
            typename View::reverse_iterator,
            typename View::const_reverse_iterator
        >::all([] <typename It> {
            constexpr bool is_const = meta::one_of<It, typename View::const_iterator, typename View::const_reverse_iterator>;

            return requires (It it, const It cit) {
                // Required typedefs
                std::is_same_v<typename It::value_type,      std::conditional_t<is_const, typename View::const_value_type, typename View::value_type>>;
                std::is_same_v<typename It::reference,       std::conditional_t<is_const, typename View::const_reference,  typename View::reference>>;
                std::is_same_v<typename It::pointer,         std::conditional_t<is_const, typename View::const_pointer,    typename View::pointer>>;
                std::is_same_v<typename It::size_type,       typename View::size_type>;
                std::is_same_v<typename It::difference_type, typename View::difference_type>;
                std::is_same_v<typename It::entity_type,     typename View::entity_type>;
                std::is_same_v<typename It::entity_traits,   typename View::entity_traits>;
                std::is_same_v<typename It::entity_utils,    typename View::entity_utils>;
                std::is_same_v<typename It::component_types, typename View::component_types>;


                // Required methods.
                { cit.entity()     } -> std::same_as<typename It::entity_type>;
                { cit.components() } -> meta::template_instantiation_of<std::tuple>;
            };
        });
    };


    /** A direct_ecs_view is an ecs_view that is directly linked to a single entity set, which can be retrieved from the view. */
    template <typename View> concept direct_ecs_view = ecs_view<View> && requires (const View cv) {
        { cv.get_entities() } -> meta::template_instantiation_of<basic_sparse_set>;
    };
}