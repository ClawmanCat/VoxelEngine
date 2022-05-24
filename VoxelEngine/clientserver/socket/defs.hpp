#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/delayed_event_dispatcher.hpp>
#include <VoxelEngine/event/subscribe_only_view.hpp>

#include <boost/asio.hpp>
#include <VoxelEngine/core/windows_header_cleanup.hpp>


namespace ve::connection {
    class socket_session;
    class socket_server;
    class socket_client;


    namespace asio = boost::asio;
    using boost::system::error_code;


    using session_id   = u32;
    using message_t    = std::vector<u8>;


    constexpr u64 message_size_limit = 64 * 1024; // 64 MB
    constexpr session_id invalid_session_id = max_value<session_id>;


    struct dispatcher_t : public subscribe_only_view<delayed_event_dispatcher<true>> {
        friend class socket_session;
    };


    struct instance_start_event {};
    struct instance_end_event   {};
    struct instance_error_event { error_code error; };

    struct session_start_event    { session_id session; };
    struct session_end_event      { session_id session; };
    struct session_error_event    { session_id session; error_code error; };
    struct message_received_event { session_id session; message_t message; };
}