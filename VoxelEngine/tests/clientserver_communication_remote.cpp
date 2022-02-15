#include <VoxelEngine/tests/test_common.hpp>
#include <VoxelEngine/tests/clientserver_communication_common.hpp>
#include <VoxelEngine/clientserver/connect.hpp>


constexpr std::size_t num_clients = 32;


template <typename Event> void log_error(const Event& event) {
    std::string message;

    if constexpr (requires { &Event::session; }) {
        message += ve::cat("[Session", event.session, "] ");
    }

    message += event.error.message();
    VE_LOG_ERROR(message);
}


test_result test_main(void) {
    using error_types = ve::meta::pack<ve::connection::instance_error_event, ve::connection::session_error_event>;


    std::size_t active_connections = 0;

    return test_with_connection_method(
        [&] (ve::client& c, ve::server& s) {
            if (active_connections == 0) {
                error_types::foreach([&] <typename T> {
                    s.template add_handler<T>(log_error<T>);
                });

                ve::host_server(s, 12000);
            }

            error_types::foreach([&] <typename T> {
                c.template add_handler<T>(log_error<T>);
            });

            ve::connect_remote(c, "127.0.0.1", 12000);
            ++active_connections;
        },

        [&] (ve::client& c, ve::server& s) {
            --active_connections;
            ve::disconnect_remote(c);

            if (active_connections == 0) {
                ve::stop_hosting_server(s);
            }
        },

        num_clients
    );
}