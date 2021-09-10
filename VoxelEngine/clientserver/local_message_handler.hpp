#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/message_handler.hpp>


namespace ve {
    class local_message_handler : public message_handler {
    public:
        template <typename Instance>
        local_message_handler(Instance& instance, instance_id remote_id, weak<local_message_handler> other = weak_nullptr) :
            message_handler(instance, remote_id)
        {
            set_other(std::move(other));
        }


        void set_other(weak<local_message_handler> other) {
            VE_ASSERT(
                other == weak_nullptr || other.lock()->get_local_id() == get_remote_id(),
                "Message handler may not switch to a different remote once it has been created."
            );

            this->other = other;
        }


        VE_GET_VAL(other);
    protected:
        void send_message(const message_t& msg) override {
            if (auto locked = other.lock(); locked) locked->on_message_received(msg);
            else VE_LOG_ERROR("Attempt to send message through local message handler with no target.");
        }

    private:
        weak<local_message_handler> other;
    };
}