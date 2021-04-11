#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/ecs/component/function_component.hpp>
#include <VoxelEngine/ecs/component/value_component.hpp>


namespace ve {
    // Differs from the runtime update component by the fact that it accepts a pointer to the static entity class instance.
    template <side Side> using static_update_component  = ve::named_function_component<"update", Side>;
    template <side Side> using static_address_component = ve::named_value_component<"static_address", void*, Side, component_serialization_mode::NONE>;
    
    
    template <side Side> class static_updater :
        public system<
            static_updater<Side>,
            Side,
            meta::pack<
                static_update_component<Side>,
                static_address_component<Side>
            >
        >
    {
    public:
        using update_component  = static_update_component<Side>;
        using address_component = static_address_component<Side>;
        
        
        static_updater(void) = default;
        
        
        void update(view_type& view, microseconds dt) {
            for (auto e : view) {
                auto [update_fn, address] = view.template get<update_component, address_component>(e);
                update_fn.template invoke_checked<void, void*, microseconds>((void*) address, dt);
            }
        }
    };
}