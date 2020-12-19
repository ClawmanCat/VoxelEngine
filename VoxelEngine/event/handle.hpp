#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/event/event_handler.hpp>

#include <boost/preprocessor.hpp>


#define ve_handle_event(dispatcher, type, capture, ...)                                 \
dispatcher.add_handler<type>(                                                           \
    ve::events::event_handler {                                                         \
        [BOOST_PP_SEQ_ELEM(0, capture)](const ve::events::event& ve_impl_event_arg) {   \
            const auto& event = static_cast<const type&>(ve_impl_event_arg);            \
            __VA_ARGS__                                                                 \
        }                                                                               \
    }                                                                                   \
)