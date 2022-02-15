#pragma once


#define ve_dereference_as(member)                                       \
constexpr auto* operator->(void) { return &member; }                    \
constexpr const auto* operator->(void) const { return &member; }        \
                                                                        \
constexpr auto& operator*(void) { return member; }                      \
constexpr const auto& operator*(void) const { return member; }