#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message_setup.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_partial_sync.hpp>


namespace ve {
    namespace detail {
        template <typename Component, typename T> void invoke_ps_callback(instance& i, entt::entity e, instance_id remote, std::span<const u8> message);


        // Performs type erasure for messaging methods in partially_synchronized.
        class partially_synchronized_type_registry {
        public:
            struct key {
                u64 component, message;

                ve_hashable();
                ve_eq_comparable(key);
            };


            static partially_synchronized_type_registry& instance(void);


            template <typename Component, typename T>
            void register_type(void) {
                callbacks[key { type_hash<Component>(), type_hash<T>() }] = [] (class instance& i, entt::entity e, instance_id remote, std::span<const u8> message) {
                    invoke_ps_callback<Component, T>(i, e, remote, message);
                };
            }

            void invoke_callback(u64 component_t, u64 message_t, class instance& i, entt::entity e, instance_id remote, std::span<const u8> message) {
                std::invoke(callbacks[key { component_t, message_t }], i, e, remote, message);
            }
        private:
            hash_map<key, fn<void, class instance&, entt::entity, instance_id, std::span<const u8>>> callbacks;
        };


        template <typename Component, typename T> struct ps_registry_helper {
            const static inline meta::null_type value = [] {
                partially_synchronized_type_registry::instance().template register_type<Component, T>();
                return meta::null_type { };
            } ();
        };


        #define ve_impl_register_ps_type(Component, Message) \
        const auto& BOOST_PP_CAT(ve_impl_hidden_var_, __LINE__) = ve::detail::ps_registry_helper<Component, Message>::value;
    }




    // Components that cannot or should not be synchronized by simply serializing the entire component when it changes
    // may instead use the partially_synchronized interface to perform synchronization.
    template <typename Derived> class partially_synchronized {
    public:
        using partially_synchronized_tag = void;


        explicit partially_synchronized(instance* owner) : owner(owner) {}


        template <typename Msg> void send_message(instance_id remote, const Msg& message) {
            ve_impl_register_ps_type(Derived, Msg);

            const static mtr_id partial_sync_msg_id = get_core_mtr_id(core_message_types::MSG_PARTIAL_SYNC);
            auto connection = get_connection(remote);

            connection->send_message(
                partial_sync_msg_id,
                partial_sync_message {
                    .message        = serialize::to_bytes(message),
                    .component_type = type_hash<Derived>(),
                    .message_type   = type_hash<Msg>(),
                    .entity         = owner->entity_for_component(static_cast<const Derived&>(*this))
                }
            );
        }


        template <typename Msg> void on_message_received(instance_id remote, const Msg& message) {
            ve_impl_register_ps_type(Derived, Msg);

            VE_CRTP_CHECK(Derived, template on_message_received<Msg>);
            static_cast<Derived*>(this)->on_message_received(remote, message);
        }
    private:
        instance* owner;


        shared<message_handler> get_connection(instance_id remote) {
            if (owner->get_type() == instance::SERVER) {
                // We could check every connection from the base class, but this is faster.
                return static_cast<server*>(owner)->get_client_connection(remote);
            } else {
                // If this is the client, there will be exactly one connection.
                // Note: we don't need to consider the case of a unified instance, since it wouldn't have any remote
                // to call this method with anyway.
                auto connection = owner->get_connections().front();
                VE_DEBUG_ASSERT(connection->get_remote_id() == remote, "Attempt to get non-existent connection.");

                return connection;
            }
        }
    };




    namespace detail {
        template <typename Component, typename T>
        inline void invoke_ps_callback(instance& i, entt::entity e, instance_id remote, std::span<const u8> message) {
            i.template get_component<Component>(e).template on_message_received<T>(remote, serialize::from_bytes<T>(message));
        }
    }
}