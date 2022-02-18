#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_partial_sync.hpp>
#include <VoxelEngine/utility/type_registry.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>


namespace ve {
    namespace detail {
        template <typename Derived, typename Msg>
        fn<void, instance&, entt::entity, std::span<const u8>, instance_id> create_invoke_on_received_fn(void);


        template <typename T1, typename T2> struct type_pair {
            using type_1 = T1;
            using type_2 = T2;
        };


        struct message_data {
            template <typename T1, typename T2> explicit message_data(meta::type_wrapper<type_pair<T1, T2>>) {
                invoke_on_received = create_invoke_on_received_fn<T1, T2>();
            }

            fn<void, instance&, entt::entity, std::span<const u8>, instance_id> invoke_on_received;
        };


        class ps_message_registry : public type_registry<
            message_data,
            /* Skip if: */ [] <typename T1, typename T2> (meta::type_wrapper<type_pair<T1, T2>>) { return !serialize::is_serializable<T2>; }
        > {
        public:
            static ps_message_registry& instance(void);
        };


        ve_impl_make_autoregister_helper(autoregister_ps_message, ps_message_registry::instance());
    }


    // For some components, it is not feasible or desirable to synchronize the entire object with the remote.
    // In this case, the partially synchronized interface can be used.
    // The interface can be used to send messages between the different versions of the component on the different instances.
    template <typename Derived> class partially_synchronizable {
        void assert_default_constructable() {
            static_assert(
                std::is_default_constructible_v<Derived>,
                "partially_synchronizable types must be default constructable so that they can be constructed on the remote instance. "
                "Alternatively, you may override Derived::from_bytes to provide a means to construct the object."
            );
        }

    public:
        using partially_synchronizable_tag = void;
        using ps_base_type                 = partially_synchronizable<Derived>;


        // partially_synchronizable class is made serializable so that it can automatically be created / destroyed using system_synchronizer.
        // No data is actually serialized by these methods.
        void to_bytes(std::vector<u8>& dst) const {}

        static Derived from_bytes(std::span<const u8>& src) {
            return Derived {};
        }


        // Automatically set the connection to send messages on when the component is used by some instance.
        void on_component_added(registry& owner, entt::entity entity) {
            // Prevent implicitly overriding this method in derived classes, since it would cause issues.
            static_assert(
                !VE_CRTP_IS_IMPLEMENTED(Derived, on_component_added),
                "on_component_added cannot be used by classes deriving from partially_synchronizable, "
                "because the function is already used by the base class. "
                "You may use the function on_component_added_wrapped instead with the same signature."
            );

            this->owner = dynamic_cast<instance*>(&owner);
            on_component_added_wrapped(owner, entity);
        }


        void on_component_added_wrapped(registry& owner, entt::entity entity) {
            VE_MAYBE_CRTP_CALL(Derived, on_component_added_wrapped, owner, entity);
        }


        // Sends the given message to the version of this component on the given remote.
        template <typename Msg> void send_message(const Msg& msg, instance_id remote) {
            VE_DEBUG_ASSERT(owner, "Cannot send message from partially_synchronized component which does not belong to any entity.");

            detail::autoregister_ps_message<detail::type_pair<Derived, Msg>>();

            owner->get_connection(remote)->send_message(
                core_message_types::MSG_PARTIAL_SYNC,
                partial_sync_message {
                    .data = serialize::to_bytes(msg),
                    .message_type = type_hash<detail::type_pair<Derived, Msg>>(),
                    .entity = entt::to_entity(owner->get_storage(), (Derived&) *this)
                }
            );
        }


        // Sends the given message to the version of this component on every remote that can see this component.
        // (As determined by the synchronization system that manages this component.)
        template <typename Msg> void broadcast_message(const Msg& msg) {
            if (!owner) return;

            detail::autoregister_ps_message<detail::type_pair<Derived, Msg>>();

            auto sync_msg = partial_sync_message {
                .data = serialize::to_bytes(msg),
                .message_type = type_hash<detail::type_pair<Derived, Msg>>(),
                .entity = entt::to_entity(owner->get_storage(), (Derived&) *this)
            };

            for (const auto& remote : get_visible_remotes()) {
                owner->get_connection(remote)->send_message(
                    core_message_types::MSG_PARTIAL_SYNC,
                    sync_msg
                );
            }
        }


        // Called when this component has been added to the given remote instance.
        // (As determined by the synchronization system that manages this component.)
        void on_added_to_remote(instance_id remote) {
            remotes.insert(remote);
            VE_MAYBE_CRTP_CALL(Derived, on_added_to_remote, remote);
        }


        // Called when this component has been removed from the given remote instance.
        // (As determined by the synchronization system that manages this component.)
        void on_removed_from_remote(instance_id remote) {
            remotes.erase(remote);
            VE_MAYBE_CRTP_CALL(Derived, on_removed_from_remote, remote);
        }


        // Called when a message is received from the version of this component on the given remote.
        template <typename Msg> void on_message_received(const Msg& msg, instance_id remote) {
            VE_MAYBE_CRTP_CALL(Derived, template on_message_received<Msg>, msg, remote);
        }


        // Get all remotes that have visibility of this component.
        hash_set<instance_id> get_visible_remotes(void) const {
            auto result = remotes;
            erase_if(result, [&] (const auto& conn) { return !owner->get_connection(conn); });
            return result;
        }


        VE_GET_VAL(owner);
    private:
        instance* owner = nullptr;
        hash_set<instance_id> remotes;
    };


    namespace detail {
        template <typename Derived, typename Msg>
        fn<void, instance&, entt::entity, std::span<const u8>, instance_id> create_invoke_on_received_fn(void) {
            return [](instance& i, entt::entity e, std::span<const u8> message, instance_id remote) {
                auto& component = i.template get_component<Derived>(e);

                component.on_message_received(
                    serialize::from_bytes<Msg>(message),
                    remote
                );
            };
        }
    }
}