#pragma once

#include <boost/preprocessor.hpp>

#include <utility>


/** Macro to forward its argument. Due to its ubiquity this macro omits the usage of the VE_ prefix. */
#define fwd(...) std::forward<decltype(__VA_ARGS__)>(__VA_ARGS__)

/** Can be used to pass a comma as part of a macro argument without it splitting the argument list. */
#define VE_COMMA ,

/** Converts the provided name to an unique identifier, based on its line in the current source file. */
#define VE_UNIQUE_NAME(name) BOOST_PP_SEQ_CAT((ve_impl_)(name)(_)(__LINE__))


/** Boolean value "true" for use within preprocessor macros. */
#define VE_TRUE 1
/** Boolean value "false" for use within preprocessor macros. */
#define VE_FALSE 0


namespace ve::detail {
    template <typename T> struct typename_wrapper {};
    template <typename T> struct typename_wrapper<void(T)> { using type = T; };
}

/** Produces a type equal to its argument, but can be used safely as a single macro argument, even if it contains a comma. */
#define VE_WRAP_TYPENAME(...) typename ve::detail::typename_wrapper<void(__VA_ARGS__)>::type