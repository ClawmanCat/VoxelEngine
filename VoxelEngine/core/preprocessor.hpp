#pragma once

#include <boost/preprocessor.hpp>

#include <utility>


#define fwd(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

// Can be used to pass a comma as part of a macro argument without it splitting the argument list.
#define ve_comma ,

// Produces a unique name for the given name so that a macro may be expanded multiple times in the same context.
// Within the same macro, ve_hide(name) expands to the same name every time.
#define ve_hide(name) BOOST_PP_SEQ_CAT((ve_impl_)(name)(_)(__LINE__))