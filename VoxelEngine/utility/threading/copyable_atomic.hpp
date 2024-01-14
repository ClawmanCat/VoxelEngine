#pragma once

#include <VoxelEngine/core/core.hpp>

#include <atomic>
#include <type_traits>


#define VE_IMPL_WRAP_CONDITIONAL_METHOD_0(Condition, Ret, Fn) \
Ret Fn(void) noexcept requires (Condition) { return std::atomic<T>::Fn(); }

#define VE_IMPL_WRAP_CONDITIONAL_METHOD_1(Condition, Ret, Fn, ArgT) \
Ret Fn(ArgT v) noexcept requires (Condition) { return std::atomic<T>::Fn(v); }


namespace ve {
    /** Wrapper around std::atomic to make it copyable/movable. */
    template <typename T> class copyable_atomic : std::atomic<T> {
    public:
        using std::atomic<T>::atomic;
        using std::atomic<T>::operator T;

        VE_IMPL_WRAP_CONDITIONAL_METHOD_0(std::is_integral_v<T> || std::is_pointer_v<T>, T, operator++);
        VE_IMPL_WRAP_CONDITIONAL_METHOD_1(std::is_integral_v<T> || std::is_pointer_v<T>, T, operator++, int);
        VE_IMPL_WRAP_CONDITIONAL_METHOD_0(std::is_integral_v<T> || std::is_pointer_v<T>, T, operator--);
        VE_IMPL_WRAP_CONDITIONAL_METHOD_1(std::is_integral_v<T> || std::is_pointer_v<T>, T, operator--, int);

        VE_IMPL_WRAP_CONDITIONAL_METHOD_1(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_pointer_v<T>, T, operator+=, T);
        VE_IMPL_WRAP_CONDITIONAL_METHOD_1(std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_pointer_v<T>, T, operator-=, T);
        VE_IMPL_WRAP_CONDITIONAL_METHOD_1(std::is_integral_v<T>, T, operator&=, T);
        VE_IMPL_WRAP_CONDITIONAL_METHOD_1(std::is_integral_v<T>, T, operator|=, T);
        VE_IMPL_WRAP_CONDITIONAL_METHOD_1(std::is_integral_v<T>, T, operator^=, T);


        constexpr copyable_atomic(T desired)                noexcept : std::atomic<T> { desired  } {}
        constexpr copyable_atomic(const std::atomic<T>& o)  noexcept : std::atomic<T> { o.load() } {}
        constexpr copyable_atomic(std::atomic<T>&& o)       noexcept : std::atomic<T> { o.load() } {}
        constexpr copyable_atomic(const copyable_atomic& o) noexcept : std::atomic<T> { o.load() } {}
        constexpr copyable_atomic(copyable_atomic&& o)      noexcept : std::atomic<T> { o.load() } {}

        constexpr T operator=(T desired)                noexcept { return std::atomic<T>::operator=(std::move(desired)); }
        constexpr T operator=(const std::atomic<T>& o)  noexcept { return std::atomic<T>::operator=(o.load());           }
        constexpr T operator=(std::atomic<T>&& o)       noexcept { return std::atomic<T>::operator=(o.load());           }
        constexpr T operator=(const copyable_atomic& o) noexcept { return std::atomic<T>::operator=(o.load());           }
        constexpr T operator=(copyable_atomic&& o)      noexcept { return std::atomic<T>::operator=(o.load());           }
    };
}