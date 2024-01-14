#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/debug/assert.hpp>

#include <magic_enum.hpp>


namespace ve::ecs::schedule {
    enum class access_mode { NONE, READ, WRITE };


    /** Returns true if it is valid to access a resource currently under access with 'current_mode' with access mode 'new_mode'. */
    [[nodiscard]] constexpr inline bool access_modes_compatible(access_mode current_mode, access_mode new_mode) {
        switch (new_mode) {
            case access_mode::NONE:  return true;
            case access_mode::READ:  return current_mode != access_mode::WRITE;
            case access_mode::WRITE: return current_mode == access_mode::NONE;
        }
    }


    /** Keeps track of how a resource is being accessed (i.e. the number of readers and writers). */
    struct access_counter {
        std::size_t readers = 0;
        bool has_writer = false;


        [[nodiscard]] access_mode current_mode(void) const {
            if (has_writer)  return access_mode::WRITE;
            if (readers > 0) return access_mode::READ;
            return access_mode::NONE;
        }


        [[nodiscard]] bool can_add_with_mode(access_mode mode) const {
            return access_modes_compatible(current_mode(), mode);
        }


        void add_with_mode(access_mode mode) {
            VE_DEBUG_ASSERT(
                can_add_with_mode(mode),
                "Invalid mode change: {} to {}.", magic_enum::enum_name(current_mode()), magic_enum::enum_name(mode)
            );

            if (mode == access_mode::WRITE) has_writer = true;
            if (mode == access_mode::READ ) ++readers;
        }


        void remove_with_mode(access_mode mode) {
            if (mode == access_mode::WRITE) has_writer = false;
            if (mode == access_mode::READ ) --readers;
        }
    };
}