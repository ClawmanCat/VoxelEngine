#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    template <typename T, typename Updater = fn<T>>
    class cache {
    public:
        explicit cache(Updater updater) : updater(std::move(updater)) {}
        ve_swap_move_only(cache, value, updater, valid);


        void invalidate(void) const {
            valid = false;
        }

        void revalidate(void) const {
            if (!std::exchange(valid, true)) value = std::invoke(updater);
        }


        // Never return mutable references: value can only be set through updater.
        operator const T&(void) const { revalidate(); return value; }
        const T& operator*(void) const { revalidate(); return value; }
        const T* operator->(void) const { revalidate(); return &value; }
        const T& get_value(void) const { revalidate(); return value; }


        VE_GET_BOOL_IS(valid);
    private:
        mutable T value;
        mutable Updater updater;
        mutable bool valid = false;
    };


    template <typename Cls, typename T, typename Updater = maybe_const_mem_fn<Cls, T>>
    class member_cache {
    public:
        member_cache(Updater updater, Cls* owner) : updater(std::move(updater)), owner(owner) {}
        ve_swap_move_only(member_cache, value, updater, valid, owner);


        void invalidate(void) const {
            valid = false;
        }

        void revalidate(void) const {
            if (!std::exchange(valid, true)) value = std::invoke(updater, owner);
        }


        // Never return mutable references: value can only be set through updater.
        operator const T&(void) const { revalidate(); return value; }
        const T& operator*(void) const { revalidate(); return value; }
        const T* operator->(void) const { revalidate(); return &value; }
        const T& get_value(void) const { revalidate(); return value; }


        VE_GET_BOOL_IS(valid);
        VE_GET_VAL(owner);
    private:
        mutable T value;
        mutable Updater updater;
        mutable bool valid = false;
        Cls* owner;
    };
}