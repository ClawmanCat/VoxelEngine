#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/change_validator.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/utility/type_registry.hpp>
#include <VoxelEngine/utility/traits/value.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>


namespace ve {
    class registry;


    namespace registry_callbacks {
        template <typename T> void set_component   (registry& r, entt::entity e, T&& v);
        template <typename T> T&   get_component   (registry& r, entt::entity e);
        template <typename T> void remove_component(registry& r, entt::entity e);
        template <typename T> std::pair<change_result, T*> set_component_checked   (instance_id remote, registry& r, entt::entity e, T&& v);
        template <typename T> std::pair<change_result, T*> remove_component_checked(instance_id remote, registry& r, entt::entity e);
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

            set_component_checked = [] (instance_id remote, registry& r, entt::entity e, std::span<const u8> v) {
                auto result = registry_callbacks::set_component_checked<T>(remote, r, e, serialize::from_bytes<T>(v));

                if (result.first == change_result::FORBIDDEN) {
                    return std::pair { result.first, serialize::to_bytes(*result.second) };
                } else {
                    return std::pair { result.first, std::vector<u8>{} };
                }
            };

            remove_component_checked = [] (instance_id remote, registry& r, entt::entity e) {
                auto result = registry_callbacks::remove_component_checked<T>(remote, r, e);

                if (result.first == change_result::FORBIDDEN) {
                    return std::pair { result.first, serialize::to_bytes(*result.second) };
                } else {
                    return std::pair { result.first, std::vector<u8>{} };
                }
            };
        }

        std::string name;
        fn<void, registry&, entt::entity, std::span<const u8>> set_component;
        fn<std::vector<u8>, registry&, entt::entity> get_component;
        fn<void, registry&, entt::entity> remove_component;

        // Note: _checked methods only return the old value of the component if the change result is forbidden.
        // This is because this is the only situation where the old value has to be sent back to the remote.
        fn<std::pair<change_result, std::vector<u8>>, instance_id, registry&, entt::entity, std::span<const u8>> set_component_checked;
        fn<std::pair<change_result, std::vector<u8>>, instance_id, registry&, entt::entity> remove_component_checked;
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