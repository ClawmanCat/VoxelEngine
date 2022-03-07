#pragma once

#include <type_traits>


#define ve_impl_bitwise_op(Enum, Op)                                                                    \
constexpr std::underlying_type_t<Enum> operator Op (Enum lhs, Enum rhs) {                               \
    return std::underlying_type_t<Enum>(lhs) Op std::underlying_type_t<Enum>(rhs);                      \
}                                                                                                       \
                                                                                                        \
constexpr std::underlying_type_t<Enum> operator Op (std::underlying_type_t<Enum> lhs, Enum rhs) {       \
    return lhs Op std::underlying_type_t<Enum>(rhs);                                                    \
}                                                                                                       \
                                                                                                        \
constexpr std::underlying_type_t<Enum> operator Op (Enum lhs, std::underlying_type_t<Enum> rhs) {       \
    return std::underlying_type_t<Enum>(lhs) Op rhs;                                                    \
}                                                                                                       \
                                                                                                        \
                                                                                                        \
constexpr Enum& operator Op##= (Enum& lhs, Enum rhs) {                                                  \
    lhs = Enum(lhs Op rhs);                                                                             \
    return lhs;                                                                                         \
}                                                                                                       \
                                                                                                        \
constexpr std::underlying_type_t<Enum>& operator Op##= (std::underlying_type_t<Enum>& lhs, Enum rhs) {  \
    lhs = lhs Op rhs;                                                                                   \
    return lhs;                                                                                         \
}                                                                                                       \
                                                                                                        \
constexpr Enum& operator Op##= (Enum& lhs, std::underlying_type_t<Enum> rhs) {                          \
    lhs = Enum(lhs Op rhs);                                                                             \
    return lhs;                                                                                         \
}



#define ve_bitwise_enum(Enum)                                                                           \
ve_impl_bitwise_op(Enum, &)                                                                             \
ve_impl_bitwise_op(Enum, |)                                                                             \
ve_impl_bitwise_op(Enum, ^)