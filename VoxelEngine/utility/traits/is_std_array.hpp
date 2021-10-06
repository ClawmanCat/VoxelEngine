#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve::meta {
    // Checks if T is a specialization of std::array.
    template <typename T> class is_std_array {
    private:
        template <typename E, std::size_t N> constexpr static auto test(const std::array<E, N>&) -> std::true_type;
        constexpr static auto test(...) -> std::false_type;
    public:
        constexpr static bool value = decltype(test(std::declval<T>()))::value;
    };
    
    
    template <typename T> constexpr inline bool is_std_array_v = is_std_array<T>::value;


    template <typename T> requires std::is_array_v<T> || is_std_array_v<T>
    constexpr inline std::size_t array_size = [] {
        if constexpr (is_std_array_v<T>) return std::tuple_size_v<T>;
        else return sizeof(T) / sizeof(*std::declval<T>());
    }();


    template <typename T> requires std::is_array_v<T> || is_std_array_v<T>
    using array_value_type = std::remove_reference_t<decltype(std::declval<T>()[0])>;
}