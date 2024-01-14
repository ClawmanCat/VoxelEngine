#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    /** Class that acts as a pointer to T, but actually contains T internally. */
    template <typename T> class fake_pointer {
    public:
        explicit fake_pointer(T value) : value(std::move(value)) {}


        [[nodiscard]] constexpr explicit operator bool(void) const { return true; }

        [[nodiscard]] constexpr T&       operator*(void)       { return *value; }
        [[nodiscard]] constexpr const T& operator*(void) const { return *value; }

        [[nodiscard]] constexpr T*       operator->(void)       { return std::addressof(value); }
        [[nodiscard]] constexpr const T* operator->(void) const { return std::addressof(value); }
    private:
        T value;
    };
}