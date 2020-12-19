#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/cached.hpp>
#include <VoxelEngine/utils/functional.hpp>


#define VE_IMPL_INVALIDATE(Rep, Data, Elem) Elem.invalidate();

#define VE_IMPL_INVALIDATE_ALL(...)                                             \
BOOST_PP_SEQ_FOR_EACH(                                                          \
    VE_IMPL_INVALIDATE,                                                         \
    _,                                                                          \
    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                       \
)

#define VE_IMPL_GET_SET_INVALIDATE_NVT(name, value, transform, ...)             \
[[nodiscard]] auto get_##name(void) const {                                     \
    return value;                                                               \
}                                                                               \
                                                                                \
void set_##name(auto&& val) {                                                   \
    VE_IMPL_INVALIDATE_ALL(__VA_ARGS__)                                         \
                                                                                \
    this->value = transform(std::forward<decltype(val)>(val));                  \
}

#define VE_IMPL_GET_SET_INVALIDATE_NV(name, value, ...) VE_IMPL_GET_SET_INVALIDATE_NVT(name, value, ve::identity, __VA_ARGS__)
#define VE_IMPL_GET_SET_INVALIDATE_V(value, ...) VE_IMPL_GET_SET_INVALIDATE_NV(value, value, __VA_ARGS__)