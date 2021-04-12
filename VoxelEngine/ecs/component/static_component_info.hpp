#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/side/side.hpp>


namespace ve {
    template <side Side, component_serialization_mode CSM>
    struct static_component_value_info {
        constexpr static inline side side                        = Side;
        constexpr static inline component_serialization_mode csm = CSM;
        constexpr static inline bool functional                  = false;
    };
    
    
    template <side Side>
    struct static_component_fn_info {
        constexpr static inline side side                        = Side;
        constexpr static inline component_serialization_mode csm = component_serialization_mode::BINARY;
        constexpr static inline bool functional                  = true;
    };
}