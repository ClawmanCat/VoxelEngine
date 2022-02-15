#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/socket/defs.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/compression.hpp>
#include <VoxelEngine/utility/io/serialize/variable_length_encoder.hpp>
#include <VoxelEngine/utility/thread/threadsafe_counter.hpp>

#include <queue>


namespace ve::connection {
    class socket_session : public dispatcher_t, public std::enable_shared_from_this<socket_session> {
    public:
        ve_shared_only(socket_session, asio::io_context& ctx, asio::ip::tcp::socket socket, shared<dispatcher_t> parent) :
            dispatcher_t { },
            std::enable_shared_from_this<socket_session> { },
            id(threadsafe_counter<"ve.connection.session">::next()),
            socket { std::move(socket) },
            strand { ctx },
            parent_dispatcher { std::move(parent) }
        {}

        ~socket_session(void) {
            stop();
            VE_ASSERT(!is_open(), "Session failed to close correctly.");

            // Note that we can't simply dispatch any remaining events when we close the socket,
            // since we want to guarantee event handlers are called from the thread that invokes owner::update().
            VE_ASSERT(!has_pending_events(), "Session was destroyed with pending events.");
        }

        ve_immovable(socket_session);


        void start(void) {
            // Note: sessions can not be restarted once they're closed.
            if (!socket.is_open()) {
                dispatch_event(session_error_event { id, asio::error::not_connected });
                return;
            }


            dispatch_event(session_start_event { id });
            do_async_read();
        }


        void stop(void) {
            if (socket.is_open()) {
                error_code error;

                socket.shutdown(asio::socket_base::shutdown_both, error);
                socket.close();

                if (error) dispatch_event(session_error_event { id, error });
                dispatch_event(session_end_event { id });

                is_closed = true;
            }
        }


        void write(message_t message) {
            // do_async_write and write need to happen on the same thread, so use a strand for dispatching.
            asio::dispatch(
                strand,
                [self = shared_from_this(), msg = std::move(message)] () {
                    auto compressed_msg = compress(msg, compression_mode::BEST_PERFORMANCE);

                    self->write_queue.push(std::move(compressed_msg));
                    self->do_async_write();
                }
            );
        }


        void update(void) {
            dispatch_events();
        }


        bool is_open(void) const {
            return !is_closed;
        }


        VE_GET_VAL(id);
        VE_GET_VAL(parent_dispatcher);
    private:
        session_id id;
        asio::ip::tcp::socket socket;
        asio::io_context::strand strand;

        shared<dispatcher_t> parent_dispatcher;

        // Since the buffer is sent out as an event twice, keeping it as a pointer prevents a copy.
        shared<message_t> read_buffer;
        message_t read_header_buffer;
        std::queue<message_t> write_queue;
        message_t write_header_buffer;

        bool is_writing = false;
        std::atomic_bool is_closed = false;


        template <typename Event> void dispatch_event(Event&& event) {
            dispatcher_t::add_event(event);
            parent_dispatcher->add_event(fwd(event));
        };


        void do_async_write(void) {
            // Already writing or nothing to write. A new write will be started if the current write (if any) is done or
            // when a new write request is issued.
            if (is_writing || write_queue.empty()) return;


            // Dispatch an error event for sending data over a closed session, since data being lost may be an issue.
            if (!is_open()) {
                dispatch_event(session_error_event { id, asio::error::not_connected });
                return;
            }


            is_writing = true;

            // Header is transferred in reverse so the last byte has its msb set, which we use to indicate the end of the header.
            write_header_buffer.clear();
            serialize::encode_variable_length(write_queue.front().size(), write_header_buffer);
            std::reverse(write_header_buffer.begin(), write_header_buffer.end());

            asio::async_write(
                socket,
                std::array<asio::const_buffer, 2> { asio::buffer(write_header_buffer), asio::buffer(write_queue.front()) },
                // do_async_write and write need to happen on the same thread, so use a strand for the callback.
                strand.wrap(ve::bind_front(&socket_session::on_async_write_complete, shared_from_this()))
            );
        }


        void on_async_write_complete(error_code error, std::size_t n) {
            if (error) [[unlikely]] {
                dispatch_event(session_error_event { id, error });
                return stop();
            }

            write_queue.pop();
            is_writing = false;

            do_async_write();
        }


        void do_async_read(void) {
            if (!is_open()) return;


            read_header_buffer.clear();

            asio::async_read(
                socket,
                asio::dynamic_buffer(read_header_buffer),
                serialize::transfer_variable_length(read_header_buffer),
                ve::bind_front(&socket_session::on_async_read_header, shared_from_this())
            );
        }


        void on_async_read_header(error_code error, std::size_t n) {
            if (error) [[unlikely]] {
                // EOF can still trigger if all data was received.
                if (error != asio::error::eof || serialize::transfer_variable_length(read_header_buffer)(false, n) != 0) {
                    dispatch_event(session_error_event { id, error });
                    return stop();
                }
            }

            // Header is transferred in reverse so the last byte has its msb set, which we use to indicate the end of the header.
            std::reverse(read_header_buffer.begin(), read_header_buffer.end());
            auto span = std::span<const u8> { read_header_buffer.begin(), read_header_buffer.end() };
            u64 message_size = serialize::decode_variable_length(span);

            if (message_size > message_size_limit) {
                dispatch_event(session_error_event { id, asio::error::message_size });
                return stop();
            }


            read_buffer = make_shared<message_t>();
            read_buffer->reserve(message_size);

            asio::async_read(
                socket,
                asio::dynamic_buffer(*read_buffer),
                asio::transfer_exactly(message_size),
                ve::bind_front(&socket_session::on_async_read_complete, shared_from_this(), message_size)
            );
        }


        void on_async_read_complete(u64 msg_size, error_code error, std::size_t n) {
            if (error) [[unlikely]] {
                // EOF can still trigger if all data was received.
                if (error != asio::error::eof || read_buffer->size() != msg_size) {
                    dispatch_event(session_error_event { id, error });
                    return stop();
                }
            }


            dispatch_event(message_received_event { id, decompress(*read_buffer) });
            return do_async_read();
        }
    };
}