#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/graphics/presentation/window.hpp>


namespace ve {
    struct input_event_base {
        gfx::window* window;
    };


    // Event bodies are often declared as { type x, y; }, which breaks for references if we don't use an alias.
    template <typename T> using cref_of = std::add_lvalue_reference_t<std::add_const_t<T>>;
}


#define ve_input_event(name) name : public input_event_base