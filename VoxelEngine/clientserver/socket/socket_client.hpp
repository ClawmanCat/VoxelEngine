#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/socket/defs.hpp>
#include <VoxelEngine/clientserver/socket/socket_session.hpp>

#include <condition_variable>


namespace ve::connection {
    class socket_client : public dispatcher_t, public std::enable_shared_from_this<socket_client> {
    public:
        ve_shared_only(socket_client) {}
        ~socket_client(void) { stop(); }
        ve_immovable(socket_client);


        void start(std::string_view address, u16 port) {
            if (!exited) {
                add_event(instance_error_event { asio::error::already_started });
                return;
            }

            exited = false;
            session = nullptr;


            asio::ip::tcp::resolver resolver { ctx };
            auto endpoint = resolver.resolve(address, to_string(port));

            auto socket = make_shared<asio::ip::tcp::socket>(ctx);
            asio::async_connect(
                *socket,
                endpoint,
                [self = weak_from_this(), socket = socket] (auto&&... args) mutable {
                    if (auto ptr = self.lock(); ptr) {
                        ptr->on_connected(std::move(socket), fwd(args)...);
                    }
                }
            );

            thread = std::thread { [&] { while (!exited) ctx.run_one(); } };


            await_connection();
            add_event(instance_start_event { });
        }


        // Note: this will dispatch any remaining events and should therefore be called from the same thread as update.
        void stop(void) {
            if (exited) return;

            exited = true;
            connection_error.assign(boost::system::errc::success, boost::system::generic_category());

            if (session) {
                session->stop();
                update();

                session = nullptr;
            }

            ctx.stop();
            thread->join();
            thread = std::nullopt;
            ctx.restart();


            add_event(instance_end_event { });
            update(); // Dispatch any remaining events.
        }


        void update(void) {
            dispatcher_t::dispatch_events();

            if (session) {
                session->update();
                if (!session->is_open()) session = nullptr;
            }
        }


        shared<socket_session> get_session(void) {
            return session;
        }
    private:
        asio::io_context ctx;
        std::optional<std::thread> thread;
        std::atomic_bool exited = true;

        shared<socket_session> session;

        error_code connection_error;
        std::mutex mtx;
        std::condition_variable cv;


        void await_connection(void) {
            std::unique_lock lock { mtx };
            cv.wait(lock, [&] { return (bool) session; });

            if (connection_error) {
                throw std::runtime_error { cat("Connection Failed: ", connection_error.message()) };
            }
        }


        void on_connected(shared<asio::ip::tcp::socket> socket, error_code error, const asio::ip::tcp::endpoint& endpoint) {
            {
                std::unique_lock lock { mtx };

                if (error) [[unlikely]] {
                    add_event(instance_error_event { error });
                    connection_error = error;
                } else {
                    session = socket_session::create(ctx, std::move(*socket), shared_from_this());
                    session->start();
                }
            }

            cv.notify_all();
        }
    };
}