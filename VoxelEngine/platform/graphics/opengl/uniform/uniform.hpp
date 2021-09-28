#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/graphics/shader/spirtype.hpp>


namespace ve::gfx::opengl {
    struct uniform {
        uniform(std::string&& name, reflect::primitive_t&& type) : name(std::move(name)), stored_type(std::move(type)) {}
        virtual ~uniform(void) = default;

        // Push combines the current value of the given uniform (if any) with the value stored in this uniform.
        // The way this is done (overwrite, add, etc.) depends on the derived class itself.
        // The result of a push is cached, and can be retrieved later using get.
        // Because of the type erasure, this is easier than storing the combination on the receiving end.
        // The engine guarantees that the type of the provided pointer is equal to the type referenced by stored_type.
        virtual const void* push(const void* current_value) const = 0;
        virtual const void* get(void) const = 0;

        std::string name;
        reflect::primitive_t stored_type;
    };


    template <typename T, typename Combine>
    class uniform_value : public uniform {
    public:
        uniform_value(std::string name, T value, Combine combine_fn) :
            uniform(std::move(name), reflect::spirtype_for<T>()), value(std::move(value)), combine_fn(std::move(combine_fn))
        {}

        const void* push(const void* current_value) const override {
            combination = combine_fn(*((const T*) current_value), value);
            return &combination;
        }

        const void* get(void) const override {
            return &combination;
        }

    private:
        Combine combine_fn;

        T value;
        mutable std::optional<T> combination = std::nullopt;
    };


    template <typename T, typename Produce, typename Combine> requires std::convertible_to<std::invoke_result_t<Produce>, T>
    class uniform_producer : public uniform {
    public:
        uniform_producer(std::string name, Produce produce_fn, Combine combine_fn) :
            uniform(std::move(name), reflect::spirtype_for<T>()), produce_fn(std::move(produce_fn)), combine_fn(std::move(combine_fn))
        {}

        const void* push(const void* current_value) const override {
            combination = combine_fn(*((const T*) current_value), produce_fn());
            return &combination;
        }

        const void* get(void) const override {
            return &combination;
        }

    private:
        mutable Produce produce_fn;
        Combine combine_fn;

        mutable std::optional<T> combination = std::nullopt;
    };


    // Deduction guide: assume T is the result of invoking Produce.
    template <typename Produce, typename Combine>
    uniform_producer(std::string, Produce, Combine) -> uniform_producer<std::invoke_result_t<Produce>, Produce, Combine>;


    namespace combine_functions {
        constexpr inline auto add       = [](const auto& old_value, const auto& new_value) { return old_value + new_value; };
        constexpr inline auto multiply  = [](const auto& old_value, const auto& new_value) { return old_value * new_value; };
        // Use perfect forwarding since uniform_producer will pass a rvalue for the new value.
        constexpr inline auto overwrite = [](const auto& old_value, auto&& new_value) -> decltype(auto) { return fwd(new_value); };
    }
}