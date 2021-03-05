#pragma once

#include <VoxelEngine/core/core.hpp>

#include <boost/preprocessor.hpp>

#include <mutex>


// Prevents concurrent calls to the function this macro is placed in.
// Macro must be placed before any critical section, usually at the top of the function.
#define ve_make_nonconcurrent                                                   \
static std::mutex BOOST_PP_CAT(ve_impl_mtx_, __LINE__) {};                      \
                                                                                \
std::lock_guard<std::mutex> BOOST_PP_CAT(ve_impl_lock_, __LINE__)               \
{ BOOST_PP_CAT(ve_impl_mtx_, __LINE__) }