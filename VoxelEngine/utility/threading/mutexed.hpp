#pragma once

#include <VoxelEngine/core/core.hpp>

#include <mutex>
#include <shared_mutex>
#include <functional>


namespace ve {
    template <typename T, typename Mutex, typename Projection>
    class basic_mutexed {
    public:
        using projected_reference       = std::invoke_result_t<Projection, T&>;
        using projected_const_reference = std::invoke_result_t<Projection, const T&>;

        basic_mutexed(void) = default;
        explicit basic_mutexed(T value, Projection projection = {}) : value(std::move(value)), projection(std::move(projection)) {}


        /** Invokes fn(value) while holding the lock in unique mode. */
        template <typename F> requires requires (Mutex m) { m.lock(); }
        decltype(auto) do_safe(F&& fn) {
            std::unique_lock lock { mutex };
            return std::invoke(fwd(fn), get_unsafe());
        }

        /** @copydoc do_safe */
        template <typename F> requires requires (Mutex m) { m.lock(); }
        decltype(auto) do_safe(F&& fn) const {
            std::unique_lock lock { mutex };
            return std::invoke(fwd(fn), get_unsafe());
        }


        /** Invokes fn(value) while holding the lock in shared mode. */
        template <typename F> requires requires (Mutex m) { m.lock_shared(); }
        decltype(auto) do_shared(F&& fn) {
            std::shared_lock lock { mutex };
            return std::invoke(fwd(fn), get_unsafe());
        }

        /** @copydoc do_shared */
        template <typename F> requires requires (Mutex m) { m.lock_shared(); }
        decltype(auto) do_shared(F&& fn) const {
            std::shared_lock lock { mutex };
            return std::invoke(fwd(fn), get_unsafe());
        }


        /** Invokes fn(value) without holding the lock. */
        template <typename F> decltype(auto) do_unsafe(F&& fn) {
            return std::invoke(fwd(fn), get_unsafe());
        }

        /** @copydoc do_unsafe */
        template <typename F> decltype(auto) do_unsafe(F&& fn) const {
            return std::invoke(fwd(fn), get_unsafe());
        }
        

        /** Returns the projected value. The mutex is not locked. */
        [[nodiscard]] projected_reference       get_unsafe(void)       { return std::invoke(projection, value); }
        /** @copydoc get_unsafe */
        [[nodiscard]] projected_const_reference get_unsafe(void) const { return std::invoke(projection, value); }
    private:
        T value;
        mutable Mutex mutex;
        VE_NO_UNIQUE_ADDRESS Projection projection;
    };


    namespace detail {
        struct dereference {
            constexpr decltype(auto) operator()(auto&& ptr) const { return *ptr; }
        };
    }


    template <typename T> using mutexed             = basic_mutexed<T,  std::mutex,        std::identity>;
    template <typename T> using shared_mutexed      = basic_mutexed<T,  std::shared_mutex, std::identity>;
    template <typename T> using mutexed_view        = basic_mutexed<T*, std::mutex,        detail::dereference>;
    template <typename T> using shared_mutexed_view = basic_mutexed<T*, std::shared_mutex, detail::dereference>;
}