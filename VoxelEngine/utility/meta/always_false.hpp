#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    template <typename T> class always_false {
    private:
        struct hidden_type { };
    public:
        constexpr static inline bool value = std::is_same_v<T, hidden_type>;
    };


    template <typename T> constexpr inline bool always_false_v = always_false<T>::value;
}