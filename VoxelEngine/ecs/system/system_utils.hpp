#pragma once

#include <VoxelEngine/core/core.hpp>


// Helper for invoking methods of some mixin base of a system.
#define ve_impl_call_mixin(Mixin, Fn, ...) \
((Mixin*) this)->Fn(static_cast<Derived&>(*this) __VA_OPT__(,) __VA_ARGS__)


namespace ve {
    // Determines what kinds of actions a system is allowed to perform on a component.
    // This is used to schedule systems concurrently in multithreaded situations.
    enum class system_access_mode : u8 {
        READ_CMP     = 0b0000'0001, // Indicates the component may be read.
        WRITE_CMP    = 0b0000'0010, // Indicates the component may be written to.
        ADD_DEL_CMP  = 0b0000'0100, // Indicates the component may be added or removed.
        ADD_DEL_ENTT = 0b0000'1000, // Indicates the entity owning the component may be added or removed.
        RW_CMP       = 0b0000'0011  // Read + Write Component
    };

    ve_bitwise_enum(system_access_mode);


    // Use create_empty_view as the RequiredComponents of a system to make it accept an empty view when it is updated.
    // Note: simply passing meta::pack<> will create a view of ALL entities.
    namespace detail { struct create_empty_view_tag {}; }
    using create_empty_view = meta::pack<detail::create_empty_view_tag>;


    // Use this if the list of accessed components of a system is simply the included components, plus the excluded components,
    // accounting for create_empty_view_tag.
    namespace detail { struct deduce_access_tag {}; }
    using deduce_component_access = meta::pack<detail::deduce_access_tag>;


    namespace detail {
        // Trait to transform a pack of mixins to their accessed components.
        template <typename M> using get_mixin_component_access = typename M::accessed_components;

        // Creates a pack of all components accessed by a system with required components R and excluded components E.
        // This is used when no accessed type list is manually provided.
        template <meta::pack_of_types R, meta::pack_of_types E>
        using accessed_types = typename R::template append_pack<E>::template erase<create_empty_view_tag>;

        template <meta::pack_of_types P> using remove_empty_view_tag = typename P::template erase<create_empty_view_tag>;
    }
}