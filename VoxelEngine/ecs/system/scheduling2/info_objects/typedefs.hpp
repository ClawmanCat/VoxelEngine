#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::ecs::schedule {
    template <typename Registry> class system_scheduler;

    using component_type = type_id_t;
    using sequence_tag   = type_id_t;
}