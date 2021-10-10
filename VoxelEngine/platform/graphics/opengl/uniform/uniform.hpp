#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/graphics/shader/object_type.hpp>
#include <VoxelEngine/graphics/shader/glsl_layout.hpp>


namespace ve::gfx::opengl {
    namespace detail {
        template <typename T> struct struct_wrapper_for_t { T value; };

        template <typename T> inline std::size_t object_hash_for(void) {
            return reflect::object_type { meta::type_wrapper<T>{} }.hash();
        }

        // Required for matching T against struct { T } for one-member UBO aliases.
        template <typename T> inline std::size_t wrapped_object_hash_for(void) {
            return reflect::object_type { meta::type_wrapper<struct_wrapper_for_t<T>>{} }.hash();
        }
    }


    struct uniform {
        template <typename T>
        uniform(std::string&& name, std::size_t size, meta::type_wrapper<T>) :
            name(std::move(name)),
            size(size),
            stored_type_hash(detail::object_hash_for<T>()),
            wrapped_type_hash(detail::wrapped_object_hash_for<T>())
        {}

        virtual ~uniform(void) = default;

        // Combines the current value of the uniform with this uniforms value in some derived-class-dependent way,
        // and stores the result in this object.
        // Note: this is a caching action so it can be considered const.
        virtual void combine(const void* current_value) const = 0;
        // Get the currently stored combined value of the uniform.
        virtual const void* get(void) const = 0;
        // Gets the above value, but in the form of a buffer with std140 layout.
        virtual void get_std140(std::vector<u8>& dest) const = 0;

        std::string name;
        std::size_t size;
        std::size_t stored_type_hash;
        std::size_t wrapped_type_hash;
    };


    template <typename T, typename Combine>
    class uniform_value : public uniform {
    public:
        uniform_value(std::string name, T value, Combine combine_fn) :
            uniform(std::move(name), sizeof(T), meta::type_wrapper<T> { }),
            combine_fn(std::move(combine_fn)),
            value(std::move(value))
        {}

        void combine(const void* current_value) const override {
            if (current_value) {
                combination = combine_fn(*((const T*) current_value), value);
            } else {
                combination = value;
            }
        }

        const void* get(void) const override {
            return &*combination;
        }

        void get_std140(std::vector<u8>& dest) const override {
            store_std140_into(*combination, dest);
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
            uniform(std::move(name), sizeof(T), meta::type_wrapper<T> { }),
            produce_fn(std::move(produce_fn)),
            combine_fn(std::move(combine_fn))
        {}

        void combine(const void* current_value) const override {
            if (current_value) {
                combination = combine_fn(*((const T*) current_value), produce_fn());
            } else {
                combination = produce_fn();
            }
        }

        const void* get(void) const override {
            return &*combination;
        }

        void get_std140(std::vector<u8>& dest) const override {
            store_std140_into(*combination, dest);
        }

    private:
        mutable Produce produce_fn;
        Combine combine_fn;

        mutable std::optional<T> combination = std::nullopt;
    };


    // Deduction guide: assume T is the result of invoking Produce.
    template <typename Produce, typename Combine>
    uniform_producer(std::string, Produce, Combine) -> uniform_producer<std::invoke_result_t<Produce>, Produce, Combine>;
}