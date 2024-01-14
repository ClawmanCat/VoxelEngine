#pragma once

#include <VoxelEngine/core/core.hpp>

#include <functional>


namespace ve {
    /**
     * Invokes the provided task when the on_scope_exit object is destroyed. Useful for achieving RAII-semantics within a function body.
     * Example usage:
     * ~~~
     * void my_function(void) {
     *     // Prevent function from being called recursively.
     *     static bool is_called = false;
     *
     *     is_called = true;
     *     on_scope_exit mark_not_called { [&] { is_called = false; } };
     *
     *     do_nonrecursive_things_that_maybe_throw();
     * }
     * ~~~
     */
    template <typename Task> class on_scope_exit {
    public:
        /** Construct an on_scope_exit object with the provided task. */
        on_scope_exit(Task task) : task(std::move(task)) {}
        /** Invokes the task contained within this on_scope_exit object. */
        ~on_scope_exit(void) { std::invoke(task); }
    private:
        Task task;
    };


    template <typename Value, typename Destroy> class raii_token {
    public:
        raii_token(void) : value(std::nullopt) {}
        explicit raii_token(Value value, Destroy destroy) : value(std::move(value)), destructor(std::move(destroy)) {}


        raii_token(const raii_token&) = delete;
        raii_token& operator=(const raii_token&) = delete;


        raii_token(raii_token&& other) noexcept { *this = std::move(other); }

        raii_token& operator=(raii_token&& other) noexcept {
            if (value) std::invoke(destructor, *value);

            value      = std::move(other.value);
            destructor = std::move(other.destructor);

            // Note: moving the optional does not leave it in a nullopt state, rather it will just contain a moved-from object.
            other.value = std::nullopt;

            return *this;
        }


        ~raii_token(void) {
            if (value) std::invoke(destructor, *value);
        }


        [[nodiscard]]       Value& operator* (void)       { return *value; }
        [[nodiscard]] const Value& operator* (void) const { return *value; }
        [[nodiscard]]       Value* operator->(void)       { return std::addressof(*value); }
        [[nodiscard]] const Value* operator->(void) const { return std::addressof(*value); }
    private:
        std::optional<Value> value;
        Destroy destructor;
    };
}