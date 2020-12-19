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
    };
}