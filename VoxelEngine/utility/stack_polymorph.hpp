#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/const_as.hpp>

#include <memory>


namespace ve {
    // Utilities for getting the largest capacity / alignment requirements for a set of types.
    template <typename... Ts> constexpr inline std::size_t common_size      = std::max({ sizeof(Ts)...  });
    template <typename... Ts> constexpr inline std::size_t common_alignment = std::max({ alignof(Ts)... });


    // Provides storage for a polymorphic object, similar to std::unique_ptr.
    // If the stored object is smaller than the given capacity, it is stored directly in the stack polymorph,
    // increasing cache locality.
    // Null values are supported, even in the case of local storage.
    template <typename Base, std::size_t Capacity, std::size_t Align = alignof(std::max_align_t)>
    class stack_polymorph {
    public:
        using base_t = Base;
        constexpr static inline std::size_t capacity  = Capacity;
        constexpr static inline std::size_t alignment = Align;


        stack_polymorph(void) noexcept = default;

        stack_polymorph(const stack_polymorph&) = delete;
        stack_polymorph& operator=(const stack_polymorph&) = delete;


        template <typename Derived> requires std::is_base_of_v<Base, Derived>
        explicit stack_polymorph(Derived&& value) noexcept {
            (*this) = fwd(value);
        }


        template <typename Derived> requires (
            // If Base isn't polymorphic, storing derived types would cause issues when deleting.
            (std::is_base_of_v<Base, Derived> && std::is_polymorphic_v<Base>) ||
            std::is_same_v<Base, Derived>
        ) stack_polymorph& operator=(Derived&& value) noexcept {
            delete_value();
            storage_flags.has_value = true;
            move_value = make_mover<Derived>();

            if constexpr (can_contain_locally<Derived>()) {
                new(&storage.local) Derived(fwd(value));
                storage_flags.is_local = true;
            } else {
                storage.remote = new Derived(fwd(value));
                storage_flags.is_local = false;
            }

            return *this;
        }


        explicit stack_polymorph(stack_polymorph&& other) noexcept {
            other.move_value(other, *this);
        }


        stack_polymorph& operator=(stack_polymorph&& other) noexcept {
            other.move_value(other, *this);
            return *this;
        }


        ~stack_polymorph(void) {
            delete_value();
        }


        void delete_value(void) {
            if (storage_flags.has_value) [[likely]] {
                storage_flags.has_value = false;

                if (storage_flags.is_local) [[likely]] {
                    ((Base*) &storage.local)->~Base();
                } else {
                    delete storage.remote;
                }
            }
        }


        bool has_value(void) const { return storage_flags.has_value; }
        bool has_local_storage(void) const { return storage_flags.is_local; }


        Base* get(void)             { return get_common(*this); }
        const Base* get(void) const { return get_common(*this); }

        Base* operator->(void)             { return get_common(*this); }
        const Base* operator->(void) const { return get_common(*this); }

        Base& operator*(void)             { return *get_common(*this); }
        const Base& operator*(void) const { return *get_common(*this); }


        template <typename T> constexpr static bool can_contain_locally(void) {
            return sizeof(T) <= Capacity && (Align % alignof(T) == 0);
        }
    private:
        union {
            typename std::aligned_storage<Capacity, Align>::type local;
            Base* remote;
        } storage;

        struct {
            bool is_local  : 1 = false;
            bool has_value : 1 = false;
        } storage_flags;

        // TODO: Consider some other way of managing value moving. Maybe store the function externally to reduce object size?
        fn<void, stack_polymorph&, stack_polymorph&> move_value;


        static auto* get_common(auto& self) {
            using ptr_t = std::add_pointer_t<meta::const_as<decltype(self), Base>>;

            if (self.storage_flags.has_value) [[likely]] {
                if (self.storage_flags.is_local) [[likely]] return (ptr_t) &self.storage.local;
                else return (ptr_t) self.storage.remote;
            }

            return (ptr_t) nullptr;
        }


        template <typename Derived> static auto make_mover(void) {
            return [] (stack_polymorph& from, stack_polymorph& to) {
                to.delete_value();
                to.move_value = from.move_value;

                if (!from.storage_flags.has_value) [[unlikely]] return;

                if constexpr (can_contain_locally<Derived>()) {
                    // Local storage: move out of the 'from' object and destroy the moved-out-of value.
                    Derived* ptr = (Derived*) &from.storage.local;
                    new (&to.storage.local) Derived(std::move(*ptr));

                    to.storage_flags = from.storage_flags;
                    from.delete_value();
                } else {
                    // Remote storage: simply copy the pointer.
                    to.storage.remote = from.storage.remote;
                    to.storage_flags  = from.storage_flags;

                    from.storage_flags.has_value = false;
                }
            };
        }
    };
}
