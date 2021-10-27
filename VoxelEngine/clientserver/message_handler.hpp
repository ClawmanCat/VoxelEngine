#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    using message_t = std::vector<u8>;


    // TODO: Eliminate unnecessary copies of message buffer when using read queue.
    class message_handler {
    public:
        template <typename Instance>
        explicit message_handler(Instance& instance, instance_id remote_id) :
            local_mtr(&instance.get_mtr()), local_id(instance.get_id()), remote_id(remote_id)
        {
            // While it should be legal to have the template constructor in the cpp file directly,
            // only GCC seems to parse the syntax for the manually specified specializations correctly.
            constructor_impl(instance);
        }


        virtual ~message_handler(void) = default;
        ve_immovable(message_handler);


        // Registering a message type before sending it is optional,
        // the message type will be registered before sending automatically if it isn't already.
        template <typename T> void register_message_type(std::string_view type_name) {
            register_message_type(type_name, type_hash<T>());
        }

        void register_message_type(std::string_view type_name, std::size_t type_hash);


        // Enqueue incoming and outgoing messages instead of sending / handling them.
        // This can be used to prevent sending of messages when the recipient isn't ready yet.
        void toggle_queue(bool enabled);


        template <typename T>
        void add_handler(std::string_view type_name, std::function<void(const T&)> handler) {
            auto it = handlers.find(type_name);

            if (it == handlers.end()) {
                std::tie(it, std::ignore) = handlers.emplace(
                    std::string { type_name },
                    make_unique<handler_data<T>>()
                );
            }

            ((handler_data<T>*) it->second.get())->handlers.push_back(std::move(handler));
        }

        // Deduce T from the function argument.
        template <typename Fn, typename T = std::remove_cvref_t<typename meta::function_traits<Fn>::arguments::head>>
        void add_handler(std::string_view type_name, Fn&& fn) {
            add_handler<T>(type_name, fwd(fn));
        }


        template <typename T>
        void send_message(std::string_view type_name, const T& data) {
            register_message_type<T>(type_name);

            message_t message_data;
            serialize::to_bytes(data, message_data);
            serialize::to_bytes(local_mtr->get_type(type_name).id, message_data);

            if (use_queue) [[unlikely]] write_queue.push_back(std::move(message_data));
            else send_message(message_data);
        }


        VE_GET_VAL(local_id);
        VE_GET_VAL(remote_id);
        VE_GET_MREF(remote_mtr);
    protected:
        struct handler_data_base {
            virtual ~handler_data_base(void) = default;
            virtual void handle(std::span<const u8>) = 0;
        };

        template <typename T> struct handler_data : public handler_data_base {
            std::vector<std::function<void(const T&)>> handlers;

            void handle(std::span<const u8> msg) override {
                // Don't bother parsing the message if there are no handlers for it.
                if (handlers.empty()) return;

                T object = serialize::from_bytes<T>(msg);
                for (const auto& handler : handlers) handler(object);
            }
        };


        hash_map<std::string, unique<handler_data_base>> handlers;
        message_type_registry* local_mtr;
        message_type_registry remote_mtr;
        instance_id local_id, remote_id;

        // Types that were already in the MTR when the message handler was created,
        // but have not been sent to the remote yet.
        hash_set<mtr_id> unregistered_local_types;

        bool use_queue = false;
        std::vector<message_t> read_queue, write_queue;


        void on_message_received(const message_t& msg);
        virtual void send_message(const message_t&) = 0;

    private:
        template <typename Instance> void constructor_impl(Instance& instance);
    };
}