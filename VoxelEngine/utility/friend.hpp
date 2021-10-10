#pragma once

#include <VoxelEngine/core/core.hpp>


#define ve_impl_friend_accessor(R, D, E) \
auto& BOOST_PP_CAT(get_, E) { return target->E; }


// Define an interface which will provide accessors (for fields passed as __VA_ARGS__) for the given target.
// The target should mark the interface as a friend struct.
#define ve_friend_interface(interface_name, target_name, ...)   \
struct interface_name {                                         \
    target_name* target;                                        \
                                                                \
    BOOST_PP_SEQ_FOR_EACH(                                      \
        ve_impl_friend_accessor,                                \
        _,                                                      \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                   \
    );                                                          \
};