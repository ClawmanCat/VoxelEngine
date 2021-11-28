#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/core_messages/core_message.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>

#include <ctti/nameof.hpp>


namespace ve {
    struct compound_message {
        std::vector<u8> data;


        template <typename T> void push_message(mtr_identifier auto type, const T& msg, message_handler* connection) {
            const auto& mtr = connection->get_local_mtr();


            // Since the message handler won't directly parse the contents of this message,
            // we need to make sure to register its constituent message types manually.
            // If we were passed an MTR ID, we can assume that it is at least registered locally, otherwise where did the ID come from?
            if constexpr (!is_mtr_id<decltype(type)>) connection->register_message_type_local(type, type_hash<T>());
            connection->register_message_type_remote(connection->resolve_local(type), type_hash<T>());


            VE_DEBUG_ASSERT(
                mtr.get_type(type).template holds<T>(),
                "Attempt to add message of type", mtr.get_type(type).name, "with data of type", ctti::nameof<T>(), "to compound message",
                "but this is not the data type associated with that MTR type."
            );


            serialize::push_serializer ser { data };

            std::size_t old_size = data.size();
            ser.push_bytes(serialize::to_bytes(msg));
            std::size_t new_size = data.size();

            ser.push(type);
            serialize::encode_variable_length(new_size - old_size, data);
        }


        void pop_message(void) {
            std::span<const u8> span { data.begin(), data.end() };
            u64 msg_size    = serialize::decode_variable_length(span);
            u64 length_size = data.size() - span.size();

            data.resize(data.size() - msg_size - length_size - sizeof(mtr_id));
        }


        void clear(void) {
            data.clear();
        }
    };


    template <typename Instance>
    inline void on_msg_compound_received(Instance& instance, message_handler& handler, const compound_message& msg) {
        std::span<const u8> span { msg.data.begin(), msg.data.end() };

        while (!span.empty()) {
            u64    msg_size = serialize::decode_variable_length(span);
            mtr_id msg_type = serialize::from_bytes<mtr_id>(span);
            auto   msg_data = span.last(msg_size);

            handler.on_message_received(msg_type, msg_data);
        }
    }


    // Message can be used to combine multiple other messages into one.
    // Remote will handle messages in the reverse order they were added to the compound message.
    const inline core_message<compound_message> msg_compound {
        .name               = core_message_types::MSG_COMPOUND,
        .direction          = message_direction::BIDIRECTIONAL,
        .on_received_client = on_msg_compound_received<client>,
        .on_received_server = on_msg_compound_received<server>
    };
}