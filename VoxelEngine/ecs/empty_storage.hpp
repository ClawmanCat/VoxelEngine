#pragma once

#include <VoxelEngine/core/core.hpp>


#define ve_impl_component_access(T, fn, ...)        \
[&] () -> decltype(auto) {                          \
    if constexpr (std::is_empty_v<T>) {             \
        fn(__VA_ARGS__);                            \
        return ve::empty_storage_for<T>();          \
    } else {                                        \
        return fn(__VA_ARGS__);                     \
    }                                               \
}()


#define ve_impl_component_access_noeval(T, fn, ...) \
[&] () -> decltype(auto) {                          \
    if constexpr (std::is_empty_v<T>) {             \
        return ve::empty_storage_for<T>();          \
    } else {                                        \
        return fn(__VA_ARGS__);                     \
    }                                               \
}()


namespace ve {
    namespace detail {
        struct empty {};
    }


    // ENTT does not store instances of empty components in the ECS.
    // This causes an inconsistency with methods like get, which return a reference to the stored component.
    // Since empty types are standard layout types and are all layout compatible, we can store an empty object in an union,
    // and just pretend it is our component.
    // (We cannot instantiate the component directly, since it may not be trivially constructable.)
    // This does mean that empty components do not have unique addresses, but it is better than the alternative.
    template <typename T> requires (!std::is_reference_v<T> && std::is_empty_v<T> && std::is_standard_layout_v<T>)
    inline T& empty_storage_for(void) {
        static union { detail::empty a; T b; } storage { .a = { } };
        return storage.b;
    }
}