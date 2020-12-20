#pragma once

#include <VoxelEngine/core/core.hpp>

#include <ctti/type_id.hpp>

#include <vector>
#include <string>


namespace ve {
    struct script_component {
        std::string name;
        
        ctti::type_id_t return_type;
        std::vector<ctti::type_id_t> arg_types;
        
        void* fn_ptr;
        
        
        template <typename Ret, typename... Args>
        script_component(std::string&& name, Fn<Ret, Args...> fn) :
            name(std::move(name)),
            return_type(ctti::type_id<Ret>()),
            arg_types({ ctti::type_id<Args>()... }),
            fn_ptr((void*) fn)
        {}
        
        
        script_component(
            std::string&& name,
            ctti::type_id_t return_type,
            std::vector<ctti::type_id_t>&& arg_types,
            void* fn_ptr
        ) :
            name(std::move(name)),
            return_type(return_type),
            arg_types(std::move(arg_types)),
            fn_ptr(fn_ptr)
        {}
    };
}