#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/clientserver/instance_id.hpp>


namespace ve {
    struct instance_pre_tick_event  { nanoseconds dt; u64 tick_count; };
    struct instance_post_tick_event { nanoseconds dt; u64 tick_count; };

    // Called *after* a remote instance has been connected.
    struct instance_connected_event {
        instance_id local, remote;
    };

    // Called *before* a remote instance is disconnected.
    struct instance_disconnected_event {
        instance_id local, remote;
    };
}