#pragma once


#define ve_impl_bin_op(cls, member, op)                                 \
constexpr auto operator op (const cls& other) const {                   \
    return this->member op other.member;                                \
}                                                                       \
                                                                        \
constexpr auto operator op (const decltype(member)& other) const {      \
    return this->member op member;                                      \
}


#define ve_impl_compound_op(cls, member, op)                            \
constexpr auto& operator op (const cls& other) {                        \
    this->member op other.member;                                       \
    return *this;                                                       \
}                                                                       \
                                                                        \
constexpr auto& operator op (const decltype(member)& other) {           \
    this->member op member;                                             \
    return *this;                                                       \
}


#define ve_impl_increment_op(cls, member, op)                           \
constexpr auto operator op (void) {                                     \
    op value;                                                           \
    return *this;                                                       \
}                                                                       \
                                                                        \
constexpr auto operator op (int) {                                      \
    cls old = *this;                                                    \
    value op;                                                           \
    return old;                                                         \
}


#define ve_impl_unary_op(cls, member, op)                               \
constexpr auto operator##op (void) {                                    \
    op value;                                                           \
    return *this;                                                       \
}


#define ve_arithmetic_as(cls, member)                                   \
ve_impl_bin_op(cls, member, +)                                          \
ve_impl_bin_op(cls, member, -)                                          \
ve_impl_bin_op(cls, member, *)                                          \
ve_impl_bin_op(cls, member, /)                                          \
ve_impl_bin_op(cls, member, %)                                          \
                                                                        \
ve_impl_compound_op(cls, member, +=)                                    \
ve_impl_compound_op(cls, member, -=)                                    \
ve_impl_compound_op(cls, member, *=)                                    \
ve_impl_compound_op(cls, member, /=)                                    \
ve_impl_compound_op(cls, member, %=)                                    \
                                                                        \
ve_impl_increment_op(cls, member, ++);                                  \
ve_impl_increment_op(cls, member, --);


#define ve_bitwise_as(cls, member)                                      \
ve_impl_bin_op(cls, member, ^)                                          \
ve_impl_bin_op(cls, member, &)                                          \
ve_impl_bin_op(cls, member, |)                                          \
ve_impl_bin_op(cls, member, <<)                                         \
ve_impl_bin_op(cls, member, >>)                                         \
                                                                        \
ve_impl_compound_op(cls, member, ^=)                                    \
ve_impl_compound_op(cls, member, &=)                                    \
ve_impl_compound_op(cls, member, |=)                                    \
ve_impl_compound_op(cls, member, <<=)                                   \
ve_impl_compound_op(cls, member, >>=)                                   \
                                                                        \
constexpr operator bool(void) const {                                   \
    return member;                                                      \
}
