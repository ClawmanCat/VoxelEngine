#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/client.hpp>
#include <VoxelEngine/clientserver/server.hpp>
#include <VoxelEngine/clientserver/local_message_handler.hpp>


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
}