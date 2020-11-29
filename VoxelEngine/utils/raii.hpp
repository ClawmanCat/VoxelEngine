#pragma once

#include <VoxelEngine/core/core.hpp>

#include <utility>


namespace ve {
    template <typename OnConstruct, typename OnDestruct>
    class raii {
    public:
        constexpr raii(OnConstruct&& on_construct, OnDestruct&& on_destruct) :
            on_destruct(std::forward<OnDestruct>(on_destruct))
        {
            on_construct();
        }
        
        constexpr ~raii(void) {
            on_destruct();
        }
    private:
        OnDestruct on_destruct;
    };
    
    
    template <typename OnDestruct>
    class conditional_raii {
    public:
        constexpr conditional_raii(OnDestruct&& on_destruct, bool active = true) :
            on_destruct(std::forward<OnDestruct>(on_destruct)), active(active)
        {}
        
        constexpr ~conditional_raii(void) {
            on_destruct();
        }
        
        void set_active(bool active) { this->active = active; }
        [[nodiscard]] bool is_active(void) const { return active; }
    private:
        OnDestruct on_destruct;
        bool active;
    };
    
    
    // Constructs a raii object which will keep target set to true for the duration of its lifetime,
    // and sets it to false afterwards.
    template <typename Boolean> requires requires (Boolean b) { b = true; b = false; }
    constexpr inline auto raii_bool(Boolean& target) {
        Boolean* tgt_pointer = &target;
    
        return raii {
            [tgt_pointer]() { (*tgt_pointer) = true; },
            [tgt_pointer]() { (*tgt_pointer) = false; }
        };
    }
}