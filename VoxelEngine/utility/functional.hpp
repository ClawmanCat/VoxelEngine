#pragma once

#include <VoxelEngine/core/core.hpp>

#include <utility>


// Commonly used predicates
#define ve_field_equals(field, value) [&](const auto& elem) { return elem.field == value; }

#define ve_equal_on_field(field)     [](const auto& a, const auto& b) { return a.field == b.field; }
#define ve_different_on_field(field) [](const auto& a, const auto& b) { return a.field != b.field; }

#define ve_get_field(field)     [&](const auto& elem) { return elem.field; }
#define ve_get_field_ref(field) [&](auto& elem) -> auto& { return elem.field; }
#define ve_tf_field(name, fn) [&](const auto& name) { return fn; }

#define ve_produce(value) [](auto... args) { return value; }

#define ve_get(x) [&](void) { return x; }
#define ve_set(x) [&](auto&& v) { x = std::forward<decltype(v)>(v); }


// Vector / Matrix operations
#define ve_vec_transform(name, change)                      \
[&] <typename T, std::size_t N> (ve::vec<N, T> vec) {       \
    for (std::size_t i = 0; i < N; ++i) {                   \
        auto name = std::move(vec[i]);                      \
        vec[i] = change;                                    \
    }                                                       \
                                                            \
    return vec;                                             \
}