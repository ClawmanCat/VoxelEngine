#pragma once

#include <utility>


#define fwd(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

// Can be used to pass a comma as part of a macro argument without it splitting the argument list.
#define ve_comma ,