#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/common_concepts.hpp>


namespace ve {
    /** Utility class to serve as an equivalent to std::unique_ptr<void> with a deleter provided at runtime. */
    class type_erased_pointer {
    public:
        type_erased_pointer(void) = default;


        template <typename T> constexpr explicit type_erased_pointer(T&& value, fn<void, void*> deleter = [] (void* v) { delete (T*) v; }) :
            value(new T { fwd(value) }),
            deleter(deleter)
        {}


        template <typename T> constexpr explicit type_erased_pointer(
            meta::type<T>,
            meta::constructor_tuple_for<T> auto constructor_args,
            fn<void, void*> deleter = [] (void* v) { delete (T*) v; }
        ) :
            value(std::apply([&] (auto&&... args) { return new T { fwd(args)... }; }, std::move(constructor_args))),
            deleter(deleter)
        {}


        type_erased_pointer(type_erased_pointer&& other) { *this = std::move(other); }


        type_erased_pointer& operator=(type_erased_pointer&& other) {
            destroy();

            value   = std::exchange(other.value, nullptr);
            deleter = other.deleter;

            return *this;
        }


        constexpr ~type_erased_pointer(void) { destroy(); }

        type_erased_pointer(const type_erased_pointer&) = delete;
        type_erased_pointer& operator=(const type_erased_pointer&) = delete;


        void* get(void) { return value; }
        const void* get(void) const { return value; }

        template <typename T> T& as(void) { return *((T*) value); }
        template <typename T> const T& as(void) const { return *((T*) value); }
    private:
        void* value = nullptr;
        fn<void, void*> deleter;


        void destroy(void) {
            if (value) [[likely]] std::invoke(deleter, value);
        }
    };
}