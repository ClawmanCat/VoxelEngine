#pragma once

#include <VoxelEngine/core/configuration.hpp>


// TODO: Handle assertion failure.
#if defined(VE_DEBUG) || defined(VE_ENABLE_ASSERTS)
    #define VE_ASSERT(...) [&]{ if (!((bool) (__VA_ARGS__))) std::terminate(); }()
#else
    #define VE_ASSERT(...) ((void) 0)
#endif