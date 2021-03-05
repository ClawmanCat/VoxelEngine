#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>
#include <VoxelEngine/utility/traits/member_function_traits.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/ecs/component/named_component.hpp>
#include <VoxelEngine/ecs/component/function_component.hpp>
#include <VoxelEngine/ecs/component/value_component.hpp>
#include <VoxelEngine/ecs/component/static_component_info.hpp>

#include <boost/preprocessor.hpp>


// IDEs will typically struggle with these macros, so simplify them to just their interface.
// Note: VE_IDE_PASS can be used as a parameter for Clangd to disable its errors for this file as well.
#if defined(__INTELLISENSE__) || defined(__JETBRAINS_IDE__) || defined(VE_IDE_PASS)
    #define VE_LOCAL_COMPONENT(name, type, ...) VE_UNWRAP(type) name
    #define VE_REMOTE_COMPONENT(name, type, ...) VE_UNWRAP(type) name
    #define VE_FUNCTION_COMPONENT(name, side, ...) name(__VA_ARGS__)
    #define VE_CONST_FUNCTION_COMPONENT(name, side, ...) name(__VA_ARGS__) const
#else


// Used for overloading macros with a variable number of arguments.
#define VE_IMPL_VARIADIC_DEFAULT(Index, Value, ...)                                         \
BOOST_PP_IF(                                                                                \
    BOOST_PP_GREATER_EQUAL(                                                                 \
        Index,                                                                              \
        BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)                                                 \
    ),                                                                                      \
    Value,                                                                                  \
    BOOST_PP_SEQ_ELEM(                                                                      \
        BOOST_PP_MIN(                                                                       \
            Index,                                                                          \
            BOOST_PP_SUB(                                                                   \
                BOOST_PP_SEQ_SIZE(                                                          \
                    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                   \
                ),                                                                          \
                1                                                                           \
            )                                                                               \
        ),                                                                                  \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                               \
    )                                                                                       \
)


#define VE_IMPL_VARIADIC_REST(Index, ...)                                                   \
BOOST_PP_SEQ_REST_N(                                                                        \
    BOOST_PP_MIN(                                                                           \
        Index,                                                                              \
        BOOST_PP_SEQ_SIZE(                                                                  \
            BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                           \
        )                                                                                   \
    ),                                                                                      \
    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                                   \
)




// Add a static component to this class.
// Variadic argument can be used to initialize the component.
// Type / serializer should be wrapped in parentheses if they contain exposed commas.
#define VE_IMPL_COMPONENT(name, type, side, csm, serializer, ...)                           \
using ve_impl_hidden_type_##name = ve::named_value_component<                               \
    #name,                                                                                  \
    VE_UNWRAP(type),                                                                        \
    side,                                                                                   \
    csm,                                                                                    \
    serializer                                                                              \
>;                                                                                          \
                                                                                            \
/* Getters and setters for property */                                                      \
const VE_UNWRAP(type)& BOOST_PP_SEQ_CAT((ve_impl_)(name)(_getter))(void) const {            \
    return get_storage()->get<ve_impl_hidden_type_##name>(get_id());                        \
}                                                                                           \
                                                                                            \
VE_UNWRAP(type)& BOOST_PP_SEQ_CAT((ve_impl_)(name)(_getter))(void) {                        \
    return get_storage()->get<ve_impl_hidden_type_##name>(get_id());                        \
}                                                                                           \
                                                                                            \
void BOOST_PP_SEQ_CAT((ve_impl_)(name)(_setter))(const VE_UNWRAP(type)& o) {                \
    get_storage()->replace<ve_impl_hidden_type_##name>(get_id(), o);                        \
}                                                                                           \
                                                                                            \
void BOOST_PP_SEQ_CAT((ve_impl_)(name)(_setter))(VE_UNWRAP(type)&& o) {                     \
    get_storage()->replace<ve_impl_hidden_type_##name>(get_id(), std::move(o));             \
}                                                                                           \
                                                                                            \
                                                                                            \
__declspec(property(                                                                        \
    get = BOOST_PP_SEQ_CAT((ve_impl_)(name)(_getter)),                                      \
    put = BOOST_PP_SEQ_CAT((ve_impl_)(name)(_setter))                                       \
)) VE_UNWRAP(type) name;                                                                    \
                                                                                            \
                                                                                            \
/* Information struct so entity can deduce if the property is present at compile time */    \
template <typename T> requires std::is_same_v<T, ve_impl_hidden_type_##name>                \
constexpr static auto ve_impl_component_info(void) {                                        \
    return ve::static_component_value_info<                                                 \
        ve_impl_this_t,                                                                     \
        VE_UNWRAP(type)                                                                     \
    > { side, csm, false, nullptr };                                                        \
}


// Calls above macro with some defaulted arguments.
// Variadic parameters can be used to provide one or more optional parameters.
// name:       The name of the variable.
// type:       The type of the variable. Should be wrapped if it contains any exposed parentheses.
// side:       The side(s) (client / server) the component exists on. Defaults to server.
// csm:        The serialization mode(s) for the component (binary / string). Defaults to binary.
// serializer: A serializer for the component. Defaults to ve::meta::null_type,
//             indicating the component should be serialized automatically.
// arguments:  Constructor arguments for the component. Defaults to empty.
#define VE_COMPONENT(name, type, ...)                                                       \
VE_IMPL_COMPONENT(                                                                          \
    name,                                                                                   \
    type,                                                                                   \
    ve::component_side::VE_IMPL_VARIADIC_DEFAULT(0, SERVER, __VA_ARGS__),                   \
    ve::component_serialization_mode::VE_IMPL_VARIADIC_DEFAULT(1, BINARY, __VA_ARGS__),     \
    VE_IMPL_VARIADIC_DEFAULT(2, ve::meta::null_type, __VA_ARGS__),                          \
    VE_IMPL_VARIADIC_REST(3, __VA_ARGS__)                                                   \
)


// Adds a static function component to this class.
// Variadic parameter should contain the arguments of the method.
// Note: this macro should be used for the declaration of the function in question. e.g:
// void VE_IMPL_FUNCTION_COMPONENT(my_fn, component_side::SERVER) { do_thing_a(); do_thing_b(); }
#define VE_IMPL_FUNCTION_COMPONENT(name, hidden_name, capture_name, side, const_kw, ...)    \
/* Macro preceeded by return type. Capture it. */                                           \
capture_name(void);                                                                         \
                                                                                            \
/* Create a wrapping function which will either call the default implementation, */         \
/* or get a function_component from the registry, if it exists. */                          \
typename ve::meta::mft<decltype(&ve_impl_this_t::capture_name)>::return_type                \
name(auto&&... args) const_kw {                                                             \
    using component_type  = ve::named_function_component<#name, side>;                      \
    using function_traits = ve::meta::mft<                                                  \
        decltype(&ve_impl_this_t::hidden_name)                                              \
    >;                                                                                      \
                                                                                            \
    if (has_dynamic_behaviour()) [[unlikely]] {                                             \
        const auto& fn = get_component<component_type>();                                   \
                                                                                            \
        return fn.invoke_unchecked<                                                         \
            function_traits::return_type                                                    \
        >(this, std::forward<decltype(args)>(args)...);                                     \
    } else {                                                                                \
        return hidden_name(std::forward<decltype(args)>(args)...);                          \
    }                                                                                       \
}                                                                                           \
                                                                                            \
                                                                                            \
/* Information struct so entity can deduce if the property is present at compile time. */   \
template <typename T> requires std::is_same_v<T, ve::named_function_component<#name, side>> \
constexpr static auto ve_impl_component_info(void) {                                        \
    return ve::static_component_fn_info { side };                                           \
}                                                                                           \
                                                                                            \
                                                                                            \
/* Default implementation. Implementation starts after macro ends. */                       \
typename ve::meta::mft<decltype(&ve_impl_this_t::capture_name)>::return_type                \
hidden_name(__VA_ARGS__) const_kw


#define VE_FUNCTION_COMPONENT(name, side, ...)                                              \
VE_IMPL_FUNCTION_COMPONENT(                                                                 \
    name,                                                                                   \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_default)),                                           \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_rt_capture)),                                        \
    ve::component_side::side,                                                               \
    /* Not Const */ __VA_OPT__(,)                                                           \
    __VA_ARGS__                                                                             \
)


#define VE_CONST_FUNCTION_COMPONENT(name, side, ...)                                        \
VE_IMPL_FUNCTION_COMPONENT(                                                                 \
    name,                                                                                   \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_default)),                                           \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_rt_capture)),                                        \
    ve::component_side::side,                                                               \
    const __VA_OPT__(,)                                                                     \
    __VA_ARGS__                                                                             \
)


#endif  // IDE Check