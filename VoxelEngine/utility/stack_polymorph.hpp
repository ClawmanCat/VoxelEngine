#pragma once

#include <VoxelEngine/core/core.hpp>

#include <memory>


namespace ve {
    // Provides storage for a polymorphic object on the stack.
    template <typename Base, std::size_t Capacity, std::size_t Align = alignof(std::max_align_t)>
    requires std::is_polymorphic_v<Base>
    struct stack_polymorph {
        template <typename Derived> constexpr static bool valid_morph = (
            std::is_base_of_v<Base, Derived> &&
            sizeof(Derived) <= Capacity      &&
            alignof(Derived) <= Align
        );


        typename std::aligned_storage<Capacity, Align>::type storage;


        template <typename Derived> requires valid_morph<Derived>
        stack_polymorph(const Derived& derived) {
            Derived* ptr = new(&storage) Derived(std::move(derived));
            return *ptr;
        }

        template <typename Derived> requires valid_morph<Derived>
        stack_polymorph(Derived&& derived) {
            Derived* ptr = new(&storage) Derived(std::move(derived));
            return *ptr;
        }

        template <typename Derived> requires valid_morph<Derived>
        Derived& operator=(const Derived& derived) {
            ((Base*) &storage)->~Base();

            Derived* ptr = new(&storage) Derived(derived);
            return *ptr;
        }

        template <typename Derived> requires valid_morph<Derived>
        Derived& operator=(Derived&& derived) {
            ((Base*) &storage)->~Base();

            Derived* ptr = new(&storage) Derived(std::move(derived));
            return *ptr;
        }


        ve_swap_move_only(stack_polymorph, storage);

        ~stack_polymorph(void) {
            ((Base*) &storage)->~Base();
        }


        operator Base&(void) { return *((Base*) &storage); }
        operator const Base&(void) const { return *((const Base*) &storage); }
    };
}
