#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/view.hpp>
#include <VoxelEngine/ecs/system/system_utils.hpp>
#include <VoxelEngine/utility/priority.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/utility/traits/pack/pack_ops.hpp>


namespace ve {
    class registry;


    template <
        typename Derived,
        meta::pack_of_types RequiredComponents,
        meta::pack_of_types ExcludedComponents = meta::pack<>,
        meta::pack_of_types AccessedComponents = deduce_component_access,
        template <typename System> typename... Mixins
    > requires (
        // Component types cannot be both required and excluded.
        meta::pack_ops::intersection<RequiredComponents, ExcludedComponents>::size == 0 &&
        (
            std::is_same_v<AccessedComponents, deduce_component_access> ||
            (
                // If a component is required or excluded, it should also be listed as accessed, since it will be contained in the view.
                AccessedComponents::template contains_all<detail::remove_empty_view_tag<RequiredComponents>> &&
                AccessedComponents::template contains_all<ExcludedComponents>
            )
        )
    ) class system : public Mixins<Derived>... {
    public:
        using derived_t           = Derived;
        using system_base_t       = system<Derived, RequiredComponents, ExcludedComponents, AccessedComponents, Mixins...>;
        using mixins              = meta::pack<Mixins<Derived>...>;
        using required_components = detail::remove_empty_view_tag<RequiredComponents>;
        using excluded_components = ExcludedComponents;
        using view_type           = registry_view_type<RequiredComponents, ExcludedComponents>;
        using ecs_system_tag      = void;


        // Deduce the list of accessed components if deduce_component_access was used.
        using accessed_components_without_mixins = std::conditional_t<
            std::is_same_v<AccessedComponents, deduce_component_access>,
            detail::accessed_types<required_components, excluded_components>,
            AccessedComponents
        >;


        // Mixins may access additional components, which needs to be accounted for.
        using accessed_components = typename accessed_components_without_mixins
            ::template append_pack<
                typename mixins::template expand_outside<detail::get_mixin_component_access>
            >;


        static auto make_view(entt::registry& storage) {
            // Note: usage of untransformed RequiredComponents is intentional here:
            // we want the empty_view_tag to be in the list, since no entity will have it.
            return view_from_registry<RequiredComponents, ExcludedComponents>(storage);
        }


        // Returns the ways in which a component may be modified by this system.
        template <typename Component> requires accessed_components::template contains<Component>
        constexpr static u8 access_mode_for_component(void) {
            if constexpr (VE_STATIC_CRTP_IS_IMPLEMENTED_TMPL(Derived, access_mode_for_component<Component>)) {
                auto derived_access = VE_STATIC_CRTP_CALL(Derived, template access_mode_for_component<Component>);

                mixins::foreach([&] <typename M> {
                    if constexpr (M::accessed_components::template contains<Component>) {
                        derived_access |= M::template access_mode_for_component<Component>();
                    }
                });

                return derived_access;
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


        // Returns true if the system modifies any external resource other than the components listed in AccessedComponents,
        // in a way that prevents concurrent execution of this system.
        // If this method returns true, this system will always be invoked on the main thread.
        constexpr static bool has_unsafe_side_effects(void) {
            if constexpr (VE_STATIC_CRTP_IS_IMPLEMENTED(Derived, has_unsafe_side_effects)) {
                auto derived = VE_STATIC_CRTP_CALL(Derived, has_unsafe_side_effects);

                mixins::foreach([&] <typename M> {
                    derived |= M::has_unsafe_side_effects();
                });

                return derived;
            } else {
                // By default assume external modifications, to prevent accidental concurrent writes.
                return true;
            }
        }


        // Returns the priority of this system compared to other systems.
        u16 get_priority(void) const {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, get_priority)) {
                return VE_CRTP_CALL(Derived, get_priority);
            } else {
                return priority::NORMAL;
            }
        }


        void on_system_added(registry& owner) {
            mixins::foreach([&] <typename M> { ve_impl_call_mixin(M, before_system_added, owner); });
            VE_MAYBE_CRTP_CALL(Derived, on_system_added, owner);
            mixins::foreach([&] <typename M> { ve_impl_call_mixin(M, after_system_added, owner); });
        }


        void on_system_removed(registry& owner) {
            mixins::foreach([&] <typename M> { ve_impl_call_mixin(M, before_system_removed, owner); });
            VE_MAYBE_CRTP_CALL(Derived, on_system_removed, owner);
            mixins::foreach([&] <typename M> { ve_impl_call_mixin(M, after_system_removed, owner); });
        }


        void on_system_update(registry& owner, view_type view, nanoseconds dt) {
            mixins::foreach([&] <typename M> { ve_impl_call_mixin(M, before_system_update, owner, view, dt); });
            VE_MAYBE_CRTP_CALL(Derived, on_system_update, owner, view, dt);
            mixins::foreach([&] <typename M> { ve_impl_call_mixin(M, after_system_update, owner, view, dt); });
        }
    };
}