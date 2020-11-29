#pragma once

#include <VoxelEngine/core/core.hpp>

#include <boost/preprocessor.hpp>


namespace ve::meta {
    struct immovable {
        immovable(void) = default;
        
        immovable(const immovable&) = delete;
        immovable(immovable&&) = delete;
        
        immovable& operator=(const immovable&) = delete;
        immovable& operator=(immovable&&) = delete;
    };
    
    #define ve_make_immovable [[no_unique_address]] ve::meta::immovable BOOST_PP_CAT(ve_impl_immovable_obj_, __LINE__)
}