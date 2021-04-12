#pragma once

#include <VoxelEngine/core/core.hpp>

#include <functional>


namespace ve {
    struct construct_stale_tag {};
    
    
    template <typename T, typename Updater = Fn<T>>
    class cached {
    public:
        template <typename UU>
        explicit cached(UU&& updater) :
            value(std::invoke(updater)),
            valid(true),
            updater(std::forward<UU>(updater))
        {}
    
    
        template <typename UU>
        explicit cached(UU&& updater, construct_stale_tag) :
            valid(false),
            updater(std::forward<UU>(updater))
        {}
        
        
        template <typename TT, typename UU>
        cached(UU&& updater, TT&& value) :
            value(std::forward<TT>(value)),
            valid(true),
            updater(std::forward<UU>(updater))
        {}
        
        
        operator T&(void) {
            validate();
            return value;
        }
        
        operator const T&(void) const {
            validate();
            return value;
        }
        
        
        T& operator*(void) { return *this; }
        const T& operator*(void) const { return *this; }
        
        T* operator->(void) { return &(**this); }
        const T* operator->(void) const { return &(**this); }
        
        
        void invalidate(void) const {
            valid = false;
        }
    private:
        mutable T value;
        mutable bool valid;
        
        [[no_unique_address]] Updater updater;
        
        
        void validate(void) const {
            if (valid) return;
            
            // Updater may do one of the following:
            // - Accept old value as parameter and return new value.
            // - Accept old value as parameter and modify it.
            // - Construct a new value without accessing the old one.
            if constexpr (std::is_invocable_r_v<Updater, T, T>) {
                value = std::invoke(updater, value);
            } else if constexpr (std::is_invocable_r_v<Updater, void, T&>) {
                std::invoke(updater, value);
            } else {
                value = std::invoke(updater);
            }
            
            valid = true;
        }
    };
    
    
    
    template <typename T, typename Cls, typename Updater = ConstMemFn<Cls, T>>
    struct member_cache {
        template <typename UU>
        member_cache(Cls* cls, UU&& updater) :
            cls(cls),
            value(std::invoke(updater, cls)),
            valid(true),
            updater(std::forward<UU>(updater))
        {}
    
    
        template <typename UU>
        member_cache(Cls* cls, UU&& updater, construct_stale_tag) :
            cls(cls),
            valid(false),
            updater(std::forward<UU>(updater))
        {}
        
        
        template <typename TT, typename UU>
        member_cache(Cls* cls, TT&& value, UU&& updater) :
            cls(cls),
            value(std::forward<TT>(value)),
            valid(true),
            updater(std::forward<UU>(updater))
        {}
        
        
        operator T&(void) {
            validate();
            return value;
        }
        
        operator const T&(void) const {
            validate();
            return value;
        }
        
        
        T& operator*(void) { return *this; }
        const T& operator*(void) const { return *this; }
        
        T* operator->(void) { return &(**this); }
        const T* operator->(void) const { return &(**this); }
        
        
        void invalidate(void) const {
            valid = false;
        }
    private:
        Cls* cls;
        
        mutable T value;
        mutable bool valid;
        
        [[no_unique_address]] Updater updater;
        
        
        void validate(void) const {
            if (valid) return;
    
            // Updater may do one of the following:
            // - Accept old value as parameter and return new value.
            // - Accept old value as parameter and modify it.
            // - Construct a new value without accessing the old one.
            if constexpr (std::is_invocable_r_v<Updater, T, Cls*, T>) {
                value = std::invoke(updater, cls, value);
            } else if constexpr (std::is_invocable_r_v<Updater, void, Cls*, T&>) {
                std::invoke(updater, cls, value);
            } else {
                value = std::invoke(updater, cls);
            }
            
            valid = true;
        }
    };
}