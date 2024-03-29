#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance.hpp>
#include <VoxelEngine/clientserver/message_handler.hpp>
#include <VoxelEngine/clientserver/message_type_registry.hpp>


namespace ve {
    class server : public instance {
    public:
        server(void) : instance(instance::SERVER) {
            initialize_mtr();
            get_validator().allow_by_default(change_result::UNOBSERVABLE);
        }


        std::string get_name(void) const override {
            return "server "s + boost::uuids::to_string(get_id());
        }


        std::vector<shared<message_handler>> get_connections(void) override {
            return client_connections | views::values | ranges::to<std::vector>;
        }


        shared<message_handler> get_connection(instance_id remote) override {
            if (auto it = client_connections.find(remote); it != client_connections.end()) return it->second;
            else return nullptr;
        }


        void add_client_connection(shared<message_handler> connection) {
            client_connections.emplace(connection->get_remote_id(), connection);
            dispatch_event(instance_connected_event { get_id(), connection->get_remote_id() });
        }


        void remove_client_connection(instance_id remote_id) {
            if (auto it = client_connections.find(remote_id); it != client_connections.end()) {
                dispatch_event(instance_disconnected_event { get_id(), remote_id });
                client_connections.erase(it);
            }
        }


        void clear_client_connections(void) {
            for (const auto& [id, connection] : client_connections) {
                dispatch_event(instance_disconnected_event { get_id(), id });
            }

            client_connections.clear();
        }


        shared<message_handler> get_client_connection(instance_id remote_id) {
            return client_connections.at(remote_id);
        }

    private:
        hash_map<instance_id, shared<message_handler>> client_connections;

        void initialize_mtr(void);
    };
}