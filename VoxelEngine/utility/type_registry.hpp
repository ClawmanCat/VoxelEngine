#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/value.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/functional.hpp>


namespace ve {
    namespace detail {
        // A lambda should be able to do this, but for some reason both Clang and GCC don't like it.
        template <auto Val> struct producer {
            constexpr decltype(Val) operator()(auto) const { return Val; }
        };
    }


    template <typename Stored, auto SkipIf = detail::producer<false>{}, auto ErrorIf = detail::producer<false>{}>
    class type_registry {
    public:
        template <typename T> void register_type(void) {
            static_assert(!ErrorIf(meta::type_wrapper<T>{}), "This type may not be registered in this registry.");

            if constexpr (!SkipIf(meta::type_wrapper<T>{})) {
                auto [it, success] = storage.emplace(
                    type_hash<T>(),
                    Stored { meta::type_wrapper<T> { } }
                );
            }
        }


        template <typename T> Stored& get(void) { return storage.at(type_hash<T>()); }
        template <typename T> const Stored& get(void) const { return storage.at(type_hash<T>()); }

        Stored& get(u64 type) { return storage.at(type); }
        const Stored& get(u64 type) const { return storage.at(type); }


        VE_GET_CREF(storage);
    private:
        hash_map<u64, Stored> storage;
    };


    // Creates an easy way to auto-register types in a given type registry.
    // E.g., given a helper with the name my_autoregister, one can write the following:
    // template <typename T> void my_fn(void) {
    //     auto register_type = my_autoregister<T>;
    //     ...
    // }
    // This will perform type registration at program startup, not when my_fn is first invoked.
    #define ve_impl_make_autoregister_helper(name, instance)                    \
    namespace detail {                                                          \
        template <typename T> struct name##_helper {                            \
            const static inline ve::meta::null_type value = [] {                \
                instance.register_type<T>();                                    \
                return ve::meta::null_type { };                                 \
            } ();                                                               \
        };                                                                      \
    }                                                                           \
                                                                                \
    template <typename T> inline void name(void) {                              \
        const auto value = detail::name##_helper<T>::value;                     \
    }
}