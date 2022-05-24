#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/message_handler.hpp>
#include <VoxelEngine/clientserver/socket/socket_session.hpp>


namespace ve {
    class remote_message_handler : public message_handler {
    public:
        template <typename Instance>
        remote_message_handler(Instance& instance, instance_id remote_id, shared<connection::socket_session> session) :
            message_handler(instance, remote_id),
            session(std::move(session))
        {
            handler_id = this->session->add_raw_handler([this] (const connection::message_received_event& e) {
                on_message_received(e.message);
            });
        }


        ~remote_message_handler(void) {
            this->session->remove_handler<connection::message_received_event>(handler_id);
            this->session->stop();
        }


        VE_GET_VAL(session);
    protected:
        void send_message(std::span<const u8> data) override {
            session->write(connection::message_t { data.begin(), data.end() });
        }

    private:
        shared<connection::socket_session> session;
        event_handler_id_t handler_id;
    };
}