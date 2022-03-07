#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/system/system.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


namespace ve {
    class registry;


    template <
        typename Derived,
        typename System,
        meta::pack_of_types AccessedComponents = meta::pack<>
    > class system_mixin {
    public:
        using derived_t           = Derived;
        using system_t            = System;
        using accessed_components = AccessedComponents;


        // Returns the ways in which a component may be modified by this mixin.
        template <typename Component> requires accessed_components::template contains<Component>
        constexpr static u8 access_mode_for_component(void) {
            if constexpr (VE_STATIC_CRTP_IS_IMPLEMENTED_TMPL(Derived, access_mode_for_component<Component>)) {
                return VE_STATIC_CRTP_CALL(Derived, template access_mode_for_component<Component>);
            } else {
                // By default assume full access, to prevent accidental concurrent writes.
                return (
                    system_access_mode::READ_CMP     |
                    system_access_mode::WRITE_CMP    |
                    system_access_mode::ADD_DEL_CMP  |
                    system_access_mode::ADD_DEL_ENTT
                );
            }
        }


        // Returns true if the mixin modifies any external resource other than the components listed in AccessedComponents,
        // in a way that prevents concurrent execution of the system containing this mixin.
        // If this method returns true, the system (and therefore this mixin) will always be invoked on the main thread.
        constexpr static bool has_unsafe_side_effects(void) {
            if constexpr (VE_STATIC_CRTP_IS_IMPLEMENTED(Derived, has_unsafe_side_effects)) {
                return VE_STATIC_CRTP_CALL(Derived, has_unsafe_side_effects);
            } else {
                // By default assume external modifications, to prevent accidental concurrent writes.
                return true;
            }
        }


        void before_system_added(System& self, registry& owner) {
            VE_MAYBE_CRTP_CALL(Derived, before_system_added, self, owner);
        }

        void after_system_added(System& self, registry& owner) {
            VE_MAYBE_CRTP_CALL(Derived, after_system_added, self, owner);
        }


        void before_system_removed(System& self, registry& owner) {
            VE_MAYBE_CRTP_CALL(Derived, before_system_removed, self, owner);
        }

        void after_system_removed(System& self, registry& owner) {
            VE_MAYBE_CRTP_CALL(Derived, after_system_removed, self, owner);
        }


        template <typename View>
        void before_system_update(System& self, registry& owner, View view, nanoseconds dt) {
            VE_MAYBE_CRTP_CALL(Derived, template before_system_update<View>, self, owner, view, dt);
        }

        template <typename View>
        void after_system_update(System& self, registry& owner, View view, nanoseconds dt) {
            VE_MAYBE_CRTP_CALL(Derived, template after_system_update<View>, self, owner, view, dt);
        }
    };
}