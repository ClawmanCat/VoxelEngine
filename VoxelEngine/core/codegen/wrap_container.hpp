#pragma once


#define ve_wrap_container(member)                                           \
using value_type      = typename decltype(member)::value_type;              \
using reference       = typename decltype(member)::reference;               \
using const_reference = typename decltype(member)::const_reference;         \
using pointer         = typename decltype(member)::pointer;                 \
using const_pointer   = typename decltype(member)::const_pointer;           \
using iterator        = typename decltype(member)::iterator;                \
using const_iterator  = typename decltype(member)::const_iterator;          \
                                                                            \
                                                                            \
auto begin(void) { return member.begin(); }                                 \
auto begin(void) const { return member.begin(); }                           \
                                                                            \
auto end(void) { return member.end(); }                                     \
auto end(void) const { return member.end(); }                               \
                                                                            \
auto size(void) const { return member.size(); }                             \
                                                                            \
decltype(auto) operator[](auto&& k) { return member[fwd(k)]; }              \
decltype(auto) operator[](auto&& k) const { return member[fwd(k)]; }        \
                                                                            \
decltype(auto) at(auto&& k) { return member.at(fwd(k)); }                   \
decltype(auto) at(auto&& k) const { return member.at(fwd(k)); }
