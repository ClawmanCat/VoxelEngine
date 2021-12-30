#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/socket/defs.hpp>
#include <VoxelEngine/clientserver/socket/socket_session.hpp>

#include <shared_mutex>


namespace ve::connection {
    class socket_server : public dispatcher_t, public std::enable_shared_from_this<socket_server> {
    public:
        ve_shared_only(socket_server, std::size_t num_threads) :
            dispatcher_t(),
            std::enable_shared_from_this<socket_server>(),
            ctx((int) num_threads),
            acceptor(ctx),
            num_threads(num_threads)
        {}

        ~socket_server(void) {
            stop();
        }

        ve_immovable(socket_server);


        void start(u16 port) {
            if (!exited) {
                add_event(instance_error_event { asio::error::already_started });
                return;
            }

            exited = false;
            sessions.clear();


            asio::ip::tcp::endpoint endpoint { asio::ip::tcp::v6(), port };

            acceptor.open(endpoint.protocol());
            acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.listen();

            accept_connection_async();


            for (std::size_t i = 0; i < num_threads; ++i) {
                threads.emplace_back([&] { while (!exited) ctx.run_one(); });
            }

            add_event(instance_start_event { });
        }


        void stop(void) {
            if (exited) return;
            exited = true;


            acceptor.close();

            {
                std::unique_lock lock { session_mtx };
                for (auto& [id, session] : sessions) session->stop();
            }

            ctx.stop();
            for (auto& thread : threads) thread.join();
            threads.clear();
            ctx.restart();


            add_event(instance_end_event { });
            update(); // Dispatch any remaining events.
        }


        // Dispatches events and destroys closed sessions.
        void update(void) {
            dispatcher_t::dispatch_events();

            {
                std::unique_lock lock { session_mtx };

                // A session could close between the point where it is updated and the point where it is removed,
                // which could lead to events being lost.
                // To fix this, update all sessions (even removed ones) after removing closed ones from the server.
                auto all_sessions = sessions | views::values | ranges::to<std::vector>;
                erase_if(sessions, [](const auto& kv) { return !kv.second->is_open(); });

                for (auto& session : all_sessions) session->update();
            }
        }


        shared<socket_session> get_session(session_id session) {
            std::shared_lock lock { session_mtx };
            return sessions.at(session);
        }


        VE_GET_CREF(sessions);
    private:
        asio::io_context ctx;
        asio::ip::tcp::acceptor acceptor;

        std::shared_mutex session_mtx;
        hash_map<session_id, shared<socket_session>> sessions;

        std::vector<std::thread> threads;
        std::size_t num_threads;
        std::atomic_bool exited = true;


        void accept_connection_async(void) {
            if (exited) return;


            auto socket = make_shared<asio::ip::tcp::socket>(ctx);
            acceptor.async_accept(*socket, [self = weak_from_this(), socket = socket] (error_code error) mutable {
                if (auto ptr = self.lock(); ptr) {
                    ptr->on_connection_accepted(std::move(socket), error);
                }
            });
        }


        void on_connection_accepted(shared<asio::ip::tcp::socket> socket, error_code error) {
            if (error) [[unlikely]] {
                add_event(instance_error_event { error });
                return accept_connection_async();
            }


            shared<socket_session> session = nullptr;

            {
                std::unique_lock lock { session_mtx };

                session = socket_session::create(ctx, std::move(*socket), shared_from_this());
                sessions.emplace(session->get_id(), session);
            }

            session->start();


            return accept_connection_async();
        }
    };
}