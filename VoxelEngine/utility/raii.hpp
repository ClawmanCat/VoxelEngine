#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/functional.hpp>


namespace ve {
    template <typename Construct = fn<void>, typename Destruct = fn<void>> class raii_function {
    public:
        constexpr raii_function(void) : destruct(no_op) { }
        constexpr raii_function(Construct&  construct, Destruct&& destruct) : destruct(fwd(destruct)) { construct(); }
        constexpr raii_function(Construct&& construct, Destruct&& destruct) : destruct(fwd(destruct)) { construct(); }
        constexpr ~raii_function(void) { destruct(); }
    
        ve_swap_move_only(raii_function, destruct);
    private:
        Destruct destruct;
    };
    
    
    template <typename T, typename Destruct = fn<T&&>> class raii_object {
    public:
        constexpr raii_object(T&& value, Destruct&& destruct) : value(fwd(value)), destruct(fwd(destruct)) {}
        constexpr ~raii_object(void) { destruct(fwd(value)); }
        
        ve_swap_move_only(raii_object, value, destruct);
        ve_dereference_as(value);
    private:
        T value;
        Destruct destruct;
    };


    class raii_tasks {
    public:
        ~raii_tasks(void) {
            for (const auto& task : tasks | views::reverse) task();
        }

        void add_task(std::function<void(void)> task) {
            tasks.push_back(std::move(task));
        }
    private:
        std::vector<std::function<void(void)>> tasks;
    };
}