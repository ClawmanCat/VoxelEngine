#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/change_validator.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>

#include <ctti/type_id.hpp>
#include <entt/entt.hpp>


namespace ve {
    class registry;


    // Implemented in registry.hpp
    namespace detail {
        template <typename T> change_result   set_component(instance_id, registry&, entt::entity, std::span<const u8>);
        template <typename T> change_result   remove_component(instance_id, registry&, entt::entity);
        template <typename T> std::vector<u8> get_component(registry&, entt::entity);
    }


    // Maps components to type IDs and provides methods of setting and removing said components from the registry.
    // Note that only serializable components are stored here, since they are the only ones that can be reconstructed from serialized data.
    class component_registry {
    public:
        struct per_component_data {
            fn<change_result, instance_id, registry&, entt::entity, std::span<const u8>> set_component;
            fn<change_result, instance_id, registry&, entt::entity> remove_component;
            fn<std::vector<u8>, registry&, entt::entity> get_component;
        };


        static component_registry& instance(void);


        template <typename T> void register_component(void) {
            components.emplace(
                type_hash<T>(),
                per_component_data {
                    .set_component    = detail::set_component<T>,
                    .remove_component = detail::remove_component<T>,
                    .get_component    = detail::get_component<T>
                }
            );
        }


        const per_component_data& get_component_data(u64 type) const {
            return components.at(type);
        }
    private:
        hash_map<u64, per_component_data> components;
    };


    namespace detail {
        template <typename Component> struct component_registry_helper {
            const static inline meta::null_type value = [] {
                if constexpr (serialize::is_serializable<Component>) {
                    component_registry::instance().template register_component<Component>();
                }
            } ();
        };
    }

    #define VE_REGISTER_COMPONENT_T(T) \
    [[maybe_unused]] const auto BOOST_PP_CAT(ve_impl_hidden_var_, __LINE__) = ve::detail::component_registry_helper<T>::value;
}