#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>


namespace ve {
    template <typename T> concept mtr_identifier = std::is_convertible_v<T, std::string_view> || std::is_same_v<T, mtr_id>;
    template <mtr_identifier T> constexpr inline bool is_mtr_id = std::is_same_v<T, mtr_id>;


    class message_handler {
    public:
        template <typename Instance>
        message_handler(Instance& instance, instance_id remote_id) :
            local_id(instance.get_id()),
            remote_id(remote_id),
            local_mtr(&instance.get_mtr())
        {
            init(instance);
        }


        virtual ~message_handler(void) = default;
        ve_immovable(message_handler);


        // Note: ID is interpreted as being a remote MTR ID, not a local one.
        template <typename T> void on_message_received(mtr_identifier auto id, const T& msg) {
            VE_DEBUG_ASSERT(
                remote_mtr.get_type(id).template holds<T>(),
                "Attempt to receive message of type", remote_mtr.get_type(id).name, "with data of type", ctti::nameof<T>(),
                "but this is not the data type associated with that MTR type."
            );


            if (use_queue) [[unlikely]] {
                std::size_t old_size = read_queue.size();
                serialize::to_bytes(msg, read_queue);
                serialize::to_bytes(resolve_remote(id), read_queue);
                std::size_t new_size = read_queue.size();

                serialize::to_bytes((u64) (new_size - old_size), read_queue);

                return;
            }


            on_message_received_common(id, &msg);
        }


        // This overload can be used to elude serialization on local connections.
        // A serializer is still required in case the handler is currently queueing messages.
        // Note: ID is interpreted as being a remote MTR ID, not a local one.
        void on_message_received(mtr_identifier auto id, const void* msg, fn<void, const void*, std::vector<u8>&> to_bytes) {
            if (use_queue) [[unlikely]] {
                std::size_t old_size = read_queue.size();
                to_bytes(msg, read_queue);
                serialize::to_bytes(resolve_remote(id), read_queue);
                std::size_t new_size = read_queue.size();

                serialize::to_bytes((u64) (new_size - old_size), read_queue);

                return;
            }


            on_message_received_common(id, msg);
        }


        // Note: ID is interpreted as being a remote MTR ID, not a local one.
        void on_message_received(mtr_identifier auto id, std::span<const u8> msg) {
            if (use_queue) [[unlikely]] {
                read_queue.insert(read_queue.end(), msg.begin(), msg.end());
                serialize::to_bytes(resolve_remote(id), read_queue);
                serialize::to_bytes((u64) (msg.size() + sizeof(mtr_id)), read_queue);

                return;
            }


            on_message_received_common(id, msg);
        }


        void on_message_received(std::span<const u8> data) {
            auto id = serialize::from_bytes<mtr_id>(data);
            on_message_received(id, data);
        }


        template <typename T> void send_message(mtr_identifier auto id, const T& value) {
            // If the ID is an MTR ID, the type must already be registered locally, otherwise where did the ID come from?
            if constexpr (!is_mtr_id<decltype(id)>) register_message_type_local(id, type_hash<T>());
            register_message_type_remote(resolve_local(id), type_hash<T>());


            VE_DEBUG_ASSERT(
                local_mtr->get_type(id).template holds<T>(),
                "Attempt to send message of type", local_mtr->get_type(id).name, "with data of type", ctti::nameof<T>(),
                "but this is not the data type associated with that MTR type."
            );


            if (use_queue) [[unlikely]] {
                std::size_t old_size = write_queue.size();
                serialize::to_bytes(value, write_queue);
                serialize::to_bytes(resolve_local(id), write_queue);
                std::size_t new_size = write_queue.size();

                serialize::to_bytes((u64) (new_size - old_size), write_queue);
                return;
            }


            send_message(
                resolve_local(id),
                &value,
                [](const void* obj, std::vector<u8>& vec) { serialize::to_bytes(*((const T*) obj), vec); }
            );
        }


        bool is_queueing(void) const {
            return use_queue;
        }


        void toggle_queue(bool enabled) {
            use_queue = enabled;


            if (!enabled) {
                // TODO: Refactor this in a way that allows us to just dispatch directly. (Currently this would happen in reverse order.)
                std::vector<std::span<const u8>> read_messages, write_messages;


                serialize::pop_deserializer read_ser { read_queue };

                while (!read_ser.empty()) {
                    auto msg_size = read_ser.pop<u64>();
                    auto msg_data = read_ser.pop_bytes(msg_size);

                    read_messages.push_back(msg_data);
                }


                serialize::pop_deserializer write_ser { write_queue };

                while (!write_ser.empty()) {
                    auto msg_size = write_ser.pop<u64>();
                    auto msg_data = write_ser.pop_bytes(msg_size);

                    write_messages.push_back(msg_data);
                }


                for (const auto& msg : read_messages | views::reverse) on_message_received(msg);
                read_queue.clear();

                for (const auto& msg : write_messages | views::reverse) send_message(msg);
                write_queue.clear();
            }
        }


        // Add a handler for the given message type.
        // The datatype for the message is automatically deduced to be the first parameter of Fn if it is not specified.
        template <typename Fn, typename T = std::remove_cvref_t<typename meta::function_traits<Fn>::arguments::head>> requires std::is_invocable_v<Fn, const T&>
        void add_handler(mtr_identifier auto id, Fn&& handler) {
            // If the ID is an MTR ID, the type must already be registered locally, otherwise where did the ID come from?
            if constexpr (!is_mtr_id<decltype(id)>) register_message_type_local(id, type_hash<T>());


            VE_DEBUG_ASSERT(
                local_mtr->get_type(id).template holds<T>(),
                "Attempt to register handler for message of type", local_mtr->get_type(id).name, "with data of type", ctti::nameof<T>(),
                "but this is not the data type associated with that MTR type."
            );


            auto resolved_id = resolve_local(id);

            auto it = handlers.find(resolved_id);
            if (it == handlers.end()) std::tie(it, std::ignore) = handlers.emplace(resolved_id, make_unique<handler_data<T>>());

            ((handler_data<T>*) it->second.get())->handlers.push_back(fwd(handler));
        }


        void register_message_type_local(std::string_view type, u64 type_hash);
        void register_message_type_remote(mtr_id type, u64 type_hash);


        // Utility for getting the actual mtr_id from a mtr_identifier.
        mtr_id resolve_local(mtr_id id) const { return id; }
        mtr_id resolve_local(std::string_view id) const { return local_mtr->get_type(id).id; }

        mtr_id resolve_remote(mtr_id id) const { return id; }
        mtr_id resolve_remote(std::string_view id) const { return remote_mtr.get_type(id).id; }


        [[nodiscard]] message_type_registry& get_local_mtr(void) { return *local_mtr; }
        [[nodiscard]] const message_type_registry& get_local_mtr(void) const { return *local_mtr; }

        [[nodiscard]] message_type_registry& get_remote_mtr(void) { return remote_mtr; }
        [[nodiscard]] const message_type_registry& get_remote_mtr(void) const { return remote_mtr; }


        VE_GET_VAL(local_id);
        VE_GET_VAL(remote_id);
    protected:
        // Override this method to perform the actual sending of data to the remote message handler.
        virtual void send_message(std::span<const u8> data) = 0;

        // Optional second method can be overridden to elude message serialization when both handlers are local.
        virtual void send_message(mtr_id id, const void* msg, fn<void, const void*, std::vector<u8>&> to_bytes) {
            std::vector<u8> data;
            to_bytes(msg, data);
            serialize::to_bytes(id, data);

            send_message(std::span<const u8> { data.begin(), data.end() });
        }

    private:
        // Implementation of the constructor must be in the CPP file to prevent a circular dependency,
        // but GCC does not handle the syntax for this correctly, so provide a wrapper method.
        template <typename Instance> void init(Instance& instance);


        // Common functionality for different overloads of on_message_received.
        void on_message_received_common(mtr_identifier auto id, const auto& value) {
            const message_type* type = nullptr;

            try {
                type = local_mtr->try_get_type(remote_mtr.get_type(id).name);
            } catch (...) {
                // Remote sent a message of an ID it hasn't registered. Just ignore it.
                VE_LOG_ERROR(cat("Received message on ", local_id, " from remote ", remote_id, " of unregistered type ", id, ". Message will be ignored."));
                return;
            }

            if (type == nullptr) return; // If the type is not known locally, we don't have any handlers for it either.
            mtr_id translated_id = type->id;

            if (auto handlers_for_msg = handlers.find(translated_id); handlers_for_msg != handlers.end()) {
                handlers_for_msg->second->handle(value);
            }
        }


        struct handler_data_base {
            virtual ~handler_data_base(void) = default;
            virtual void handle(std::span<const u8>) const = 0;
            virtual void handle(const void*) const = 0;
        };


        template <typename T> struct handler_data : handler_data_base {
            std::vector<std::function<void(const T&)>> handlers;


            void handle(std::span<const u8> msg) const override {
                T value = serialize::from_bytes<T>(msg);
                handle(&value);
            }

            void handle(const void* msg) const override {
                const T* msg_ptr = (const T*) msg;
                for (const auto& handler : handlers) handler(*msg_ptr);
            }
        };


        hash_map<mtr_id, unique<handler_data_base>> handlers;

        instance_id local_id, remote_id;
        // The local MTR contains message IDs used when this handler sends a message.
        message_type_registry* local_mtr;
        // The remote MTR contains message IDs used when the remote handler sends a message.
        message_type_registry remote_mtr;

        // Messages may be enqueued in the handler and sent at a later date.
        // This is useful when creating multiple message handlers in the same application,
        // as it can be used to prevent the exchange of messages before all handlers have been initialized.
        bool use_queue = false;
        std::vector<u8> read_queue, write_queue;

        // Since the local MTR may be shared between multiple message handlers, we can't simply assume every type in there
        // is known to the remote, even if we synchronize every time we modify the MTR. Therefore, keep track of which
        // types are known on the remote manually.
        // Note that this does not simply correspond to the contents of the remote MTR, as types are only registered there
        // if they are sent by the remote, and they will have the remote's ID mapping anyway, making them useless for this purpose.
        hash_set<mtr_id> published_types;
    };
}