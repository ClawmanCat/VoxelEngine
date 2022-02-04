#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/type_registry.hpp>
#include <VoxelEngine/utility/traits/value.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>


namespace ve {
    class registry;


    namespace registry_callbacks {
        template <typename T> void set_component   (registry& r, entt::entity e, T&& v);
        template <typename T> T&   get_component   (registry& r, entt::entity e);
        template <typename T> void remove_component(registry& r, entt::entity e);
    }


    struct component_registry_data {
        template <typename T> explicit component_registry_data(meta::type_wrapper<T>) {
            name = ctti::nameof<T>().cppstring();

            set_component = [] (registry& r, entt::entity e, std::span<const u8> v) {
                registry_callbacks::set_component<T>(r, e, serialize::from_bytes<T>(v));
            };

            get_component = [] (registry& r, entt::entity e) {
                return serialize::to_bytes(registry_callbacks::get_component<T>(r, e));
            };

            remove_component = [] (registry& r, entt::entity e) {
                registry_callbacks::remove_component<T>(r, e);
            };
        }

        std::string name;
        fn<void, registry&, entt::entity, std::span<const u8>> set_component;
        fn<std::vector<u8>, registry&, entt::entity> get_component;
        fn<void, registry&, entt::entity> remove_component;
    };


    class component_registry : public type_registry<
        component_registry_data,
        /* Skip if: */ [] <typename T> (meta::type_wrapper<T>) { return !serialize::is_serializable<T>; }
    > {
    public:
        static component_registry& instance(void);
    };


    ve_impl_make_autoregister_helper(autoregister_component, component_registry::instance());
}