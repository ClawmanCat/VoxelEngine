#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/uniform/uniform_combine_function.hpp>


namespace ve::gfx {
    template <typename Derived, typename Uniform, typename Producer = fn<Uniform, const Uniform&, const Uniform&>>
    struct uniform_convertible {
        using uniform_convertible_tag = void;
        using uniform_t               = Uniform;


        // Note: value must be constant on a per-instance basis.
        std::string get_uniform_name(void) const {
            VE_CRTP_CHECK(Derived, get_uniform_name);
            return static_cast<const Derived*>(this)->get_uniform_name();
        }

        Uniform get_uniform_value(void) const {
            VE_CRTP_CHECK(Derived, get_uniform_value);
            return static_cast<const Derived*>(this)->get_uniform_value();
        }


        // Note: value must be constant on a per-instance basis.
        Producer get_uniform_combine_function(void) const {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, get_uniform_combine_function)) {
                return static_cast<const Derived*>(this)->get_uniform_combine_function();
            } else {
                return combine_functions::overwrite;
            }
        }
    };
}