#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/message_handler.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>


namespace ve {
    class server : public instance {
    public:
        server(void) : instance() {
            initialize_mtr();
        }


        std::string get_name(void) const override {
            return "server "s + boost::uuids::to_string(get_id());
        }


        void add_client_connection(shared<message_handler> connection) {
            client_connections.emplace(connection->get_remote_id(), connection);
            dispatch_event(instance_connected_event { get_id(), connection->get_remote_id() });
        }


        void remove_client_connection(instance_id remote_id) {
            if (auto it = client_connections.find(remote_id); it != client_connections.end()) {
                dispatch_event(instance_disconnected_event { get_id(), it->second->get_remote_id() });
                client_connections.erase(it);
            }
        }


        shared<message_handler> get_client_connection(instance_id remote_id) {
            return client_connections.at(remote_id);
        }


        VE_GET_MREF(mtr);
    private:
        hash_map<instance_id, shared<message_handler>> client_connections;
        message_type_registry mtr;

        void initialize_mtr(void);
    };
}