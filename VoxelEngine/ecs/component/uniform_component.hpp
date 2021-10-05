#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // A CRTP interface for classes that map components to uniforms to be used by the renderer.
    // See transform_component for an implementation example.
    template <typename Derived, typename Component, typename Uniform>
    struct uniform_component {
        using component_t = Component;
        using uniform_t   = Uniform;


        Uniform value(const Component& component) const {
            VE_CRTP_CHECK(Derived, value);
            return static_cast<const Derived*>(this)->value(component);
        }

        std::string name(const Component& component) const {
            VE_CRTP_CHECK(Derived, name);
            return static_cast<const Derived*>(this)->name(component);
        }
    };
}