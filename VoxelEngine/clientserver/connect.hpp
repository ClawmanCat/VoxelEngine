#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance_events.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/local_message_handler.hpp>
#include <VoxelEngine/clientserver/remote_message_handler.hpp>
#include <VoxelEngine/clientserver/socket/socket_client.hpp>
#include <VoxelEngine/clientserver/socket/socket_server.hpp>
#include <VoxelEngine/clientserver/socket/socket_session.hpp>
#include <VoxelEngine/clientserver/core_messages/msg_ignore_this.hpp>
#include <VoxelEngine/utility/io/serialize/binary_serializable.hpp>


// TODO: Force usage of shared_ptr for client and server.
namespace ve {
    // Connect a client and a server running within the same application.
    inline void connect_local(client& c, server& s) {
        auto client_msg_handler = make_shared<local_message_handler>(c, s.get_id());
        client_msg_handler->toggle_queue(true);

        auto server_msg_handler = make_shared<local_message_handler>(s, c.get_id());
        server_msg_handler->toggle_queue(true);


        client_msg_handler->set_other(server_msg_handler);
        server_msg_handler->set_other(client_msg_handler);

        c.set_server_connection(client_msg_handler);
        s.add_client_connection(server_msg_handler);


        client_msg_handler->toggle_queue(false);
        server_msg_handler->toggle_queue(false);
    }




    // Disconnect a client and a server connected through connect_local.
    inline void disconnect_local(client& c, server& s) {
        s.remove_client_connection(c.get_id());
        c.remove_server_connection();
    }




    // Connect a local client to a remote server.
    inline void connect_remote(client& c, std::string_view address, u16 port) {
        auto& connection = c.store_object(
            "ve.connection",
            connection::socket_client::create()
        );

        // Connection will dispatch events after every tick of the client.
        c.store_object(
            "ve.connection.connection_update_handler",
            c.add_handler([connection] (const instance_post_tick_event& e) {
                connection->update();
            })
        );


        // Once we receive the remote's ID, create the connection.
        // This will occur after this method has exited, but the client is immovable, so taking its address shouldn't be an issue.
        connection->add_one_time_handler([connection, &c] (const connection::message_received_event& e) {
            auto span      = make_message_unignored(e.message);
            auto remote_id = serialize::from_bytes<instance_id>(span);

            c.set_server_connection(make_shared<remote_message_handler>(c, remote_id, connection->get_session()));
        });


        // Start the session and send our ID to the remote.
        connection->start(address, port);

        auto id_buffer = serialize::to_bytes(c.get_id());
        make_message_ignored(id_buffer);

        connection->get_session()->write(std::move(id_buffer));
    }




    // Disconnect a client from a remote server.
    inline void disconnect_remote(client& c) {
        auto update_handler = c.take_object<event_handler_id_t>("ve.connection.connection_update_handler");
        auto connection     = c.take_object<shared<connection::socket_client>>("ve.connection");

        c.remove_handler<instance_post_tick_event>(update_handler);
        c.remove_server_connection();

        connection->stop();
    }




    // Set up the server to accept connections from remote clients.
    inline void host_server(server& s, u16 port, std::size_t num_threads = 32) {
        auto connection = s.store_object(
            "ve.connection",
            connection::socket_server::create(num_threads)
        );

        // Connection will dispatch events after every tick of the server.
        s.store_object(
            "ve.connection.connection_update_handler",
            s.add_handler([connection] (const instance_post_tick_event& e) {
                connection->update();
            })
        );


        // When a connection is made, send out our ID and wait for the remote to do the same.
        // After this has been done, the message handler can be created.
        connection->add_handler([connection, &s] (const connection::session_start_event& e) {
            auto session = connection->get_session(e.session);

            session->add_one_time_handler([session, &s] (const connection::message_received_event& e) mutable {
                auto span      = make_message_unignored(e.message);
                auto remote_id = serialize::from_bytes<instance_id>(span);

                session->add_handler([remote_id, &s] (const connection::session_end_event& e) {
                    s.remove_client_connection(remote_id);
                });

                s.add_client_connection(make_shared<remote_message_handler>(s, remote_id, std::move(session)));
            });


            auto id_buffer = serialize::to_bytes(s.get_id());
            make_message_ignored(id_buffer);

            session->write(std::move(id_buffer));
        });


        connection->start(port);
    }




    // Stop accepting remote clients on this server and disconnect all existing ones.
    inline void stop_hosting_server(server& s) {
        auto update_handler = s.take_object<event_handler_id_t>("ve.connection.connection_update_handler");
        auto connection     = s.take_object<shared<connection::socket_server>>("ve.connection");

        s.remove_handler<instance_post_tick_event>(update_handler);
        s.clear_client_connections();

        connection->stop();
    }
}