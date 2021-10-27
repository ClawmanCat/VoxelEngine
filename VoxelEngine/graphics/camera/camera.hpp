#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/camera/camera_uniform.hpp>
#include <VoxelEngine/graphics/uniform/uniform_convertible.hpp>


namespace ve::gfx {
    template <typename Derived> struct camera : public uniform_convertible<Derived, camera_uniform> {
        using camera_tag = void;


        vec3f get_position(void) const {
            VE_CRTP_CHECK(Derived, get_position);
            return static_cast<const Derived*>(this)->get_position();
        }

        void set_position(const vec3f& pos) {
            VE_CRTP_CHECK(Derived, set_position);
            static_cast<Derived*>(this)->set_position();
        }


        quatf get_rotation(void) const {
            VE_CRTP_CHECK(Derived, get_rotation);
            return static_cast<const Derived*>(this)->get_rotation();
        }

        void set_rotation(const quatf& pos) {
            VE_CRTP_CHECK(Derived, set_rotation);
            static_cast<Derived*>(this)->set_rotation();
        }
    };
}