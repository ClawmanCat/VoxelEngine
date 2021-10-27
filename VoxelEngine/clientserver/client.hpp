#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/message_handler.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>


namespace ve {
    class client : public instance {
    public:
        client(void) : instance() {
            initialize_mtr();
        }


        std::string get_name(void) const override {
            return "client "s + boost::uuids::to_string(get_id());
        }


        void set_server_connection(shared<message_handler>&& connection) {
            if (server_connection) {
                dispatch_event(instance_disconnected_event { get_id(), server_connection->get_remote_id() });
            }

            server_connection = std::move(connection);

            if (server_connection) {
                dispatch_event(instance_connected_event { get_id(), server_connection->get_remote_id() });
            }
        }


        void remove_server_connection(void) {
            set_server_connection(nullptr);
        }


        shared<message_handler> get_server_connection(void) {
            return server_connection;
        }


        VE_GET_MREF(mtr);
    private:
        shared<message_handler> server_connection = nullptr;
        message_type_registry mtr;

        void initialize_mtr(void);
    };
}