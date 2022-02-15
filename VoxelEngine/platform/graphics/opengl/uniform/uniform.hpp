#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/value.hpp>
#include <VoxelEngine/graphics/shader/object_type.hpp>
#include <VoxelEngine/graphics/shader/glsl_layout.hpp>


namespace ve::gfx::opengl {
    namespace detail {
        template <typename T> struct wrapper { T value; };

        template <typename T> inline std::size_t type_hash_for(void) {
            return reflect::object_type { meta::type_wrapper<T>{} }.hash();
        }
    }


    // TODO: Support assignment to reduce heap allocations when changing uniforms.
    class uniform {
    public:
        template <typename T> uniform(std::string name, meta::type_wrapper<T> type) :
            name(std::move(name)),
            self_type_hash(detail::type_hash_for<T>()),
            wrapped_type_hash(detail::type_hash_for<detail::wrapper<T>>())
        {}

        virtual ~uniform(void) = default;


        // Given the value of some other uniform with the same stored type as the current object,
        // combine the value of that uniform with that of this one, using some derived-class-dependent method,
        // and return the result in a pointer, which shall remain valid until the next call to combine.
        virtual const void* combine(const void* current_value) const = 0;
        // Given a value of the same type as described above, convert it to an object with STD140 layout.
        virtual std::vector<u8> to_std140(const void* current_value) const = 0;


        VE_GET_CREF(name);
        VE_GET_VAL(self_type_hash);
        VE_GET_VAL(wrapped_type_hash);
    private:
        std::string name;
        std::size_t self_type_hash;
        std::size_t wrapped_type_hash;
    };


    template <typename T, typename Combine>
    class uniform_value : public uniform {
    public:
        uniform_value(std::string name, T value, Combine combine) :
            uniform(std::move(name), meta::type_wrapper<T>{}),
            value(std::move(value)),
            combine_fn(std::move(combine))
        {}


        const void* combine(const void* current_value) const override {
            combination = current_value
                ? std::invoke(combine_fn, *((const T*) current_value), value)
                : value;

            return &*combination;
        }


        virtual std::vector<u8> to_std140(const void* current_value) const override {
            return gfx::to_std140(*((const T*) current_value));
        }
    private:
        T value;
        mutable std::optional<T> combination;
        Combine combine_fn;
    };


    template <typename T, typename Produce, typename Combine>
    class uniform_producer : public uniform {
    public:
        uniform_producer(std::string name, Produce produce, Combine combine) :
            uniform(std::move(name), meta::type_wrapper<T>{}),
            produce_fn(std::move(produce)),
            combine_fn(std::move(combine))
        {}


        const void* combine(const void* current_value) const override {
            combination = current_value
                ? std::invoke(combine_fn, *((const T*) current_value), std::invoke(produce_fn))
                : std::invoke(produce_fn);

            return &*combination;
        }


        virtual std::vector<u8> to_std140(const void* current_value) const override {
            return gfx::to_std140(*((const T*) current_value));
        }
    private:
        mutable std::optional<T> combination;
        Produce produce_fn;
        Combine combine_fn;
    };
}