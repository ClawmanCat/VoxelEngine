#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/side/side.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/ecs/component/named_component.hpp>
#include <VoxelEngine/ecs/component/function_component.hpp>
#include <VoxelEngine/ecs/component/value_component.hpp>
#include <VoxelEngine/ecs/component/static_component_info.hpp>
#include <VoxelEngine/ecs/component/static_component_helpers.hpp>
#include <VoxelEngine/utility/variadic_macro.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>
#include <VoxelEngine/utility/traits/member_function_traits.hpp>


// IDEs and other parsing tools may struggle with these macros, so just provide fake implementations that are simpler to parse.
// Notably, a lot of tools will fail to parse __VA_OPT__ correctly, as it is a relatively new feature.
// For tools like clang-tidy, add VE_IDE_PASS to the list of preprocessor definitions to use these simplified definitions.
#if defined(__INTELLISENSE__) || defined(JETBRAINS_IDE) || defined(VE_IDE_PASS)
    #define VE_COMPONENT(name, ...) name
    #define VE_PROPER_COMPONENT(name, ...) name
    
    #define VE_FUNCTION_COMPONENT(name, side, ...) name(__VA_ARGS__)
    #define VE_CONST_FUNCTION_COMPONENT(name, side, ...) name(__VA_ARGS__) const
#else


// Implementation detail. Call VE_COMPONENT or VE_PROPER_COMPONENT to use this macro.
// TODO: Getters & Setters should be no-ops if the side does not match the registry side.
#define VE_IMPL_COMPONENT(name, hidden_type, ret_type, side, csm, serializer)                               \
/* Macro preceeded by return type. Capture it. */                                                           \
BOOST_PP_SEQ_CAT((ve_impl_)(name)(_rt_capture))(void);                                                      \
using ret_type = typename ve::meta::mft<                                                                    \
    decltype(&most_derived_t::BOOST_PP_SEQ_CAT((ve_impl_)(name)(_rt_capture)))                              \
>::return_type;                                                                                             \
                                                                                                            \
                                                                                                            \
/* Typedef for the actual type of the component. */                                                         \
/* Note: this will result in a named_value_component unless the type is already a component. */             \
using hidden_type = ve::detail::underlying_component_t<                                                     \
    #name,                                                                                                  \
    ret_type,                                                                                               \
    side,                                                                                                   \
    csm,                                                                                                    \
    serializer                                                                                              \
>;                                                                                                          \
                                                                                                            \
                                                                                                            \
/* Getters and setters for property. */                                                                     \
const ret_type& BOOST_PP_SEQ_CAT((ve_impl_)(name)(_getter))(void) const {                                   \
    return get_storage().get<hidden_type>(get_id());                                                        \
}                                                                                                           \
                                                                                                            \
ret_type& BOOST_PP_SEQ_CAT((ve_impl_)(name)(_getter))(void) {                                               \
    return get_storage().get<hidden_type>(get_id());                                                        \
}                                                                                                           \
                                                                                                            \
void BOOST_PP_SEQ_CAT((ve_impl_)(name)(_setter))(ret_type&& o) {                                            \
    get_storage().replace<hidden_type>(get_id(), std::move(o));                                             \
}                                                                                                           \
                                                                                                            \
                                                                                                            \
/* Property to emulate class member syntax. */                                                              \
__declspec(property(                                                                                        \
    get = BOOST_PP_SEQ_CAT((ve_impl_)(name)(_getter)),                                                      \
    put = BOOST_PP_SEQ_CAT((ve_impl_)(name)(_setter))                                                       \
)) ret_type name;                                                                                           \
                                                                                                            \
                                                                                                            \
/* Information struct so entity can deduce if the property is present at compile time */                    \
template <typename T> requires std::is_same_v<T, hidden_type>                                               \
constexpr static auto ve_impl_component_info(void) {                                                        \
    return ve::static_component_value_info<side, csm> { };                                                  \
}                                                                                                           \
                                                                                                            \
                                                                                                            \
/* Add component to registry on entity construction with provided arguments. */                             \
/* See definition of assignment_helper for information on how this works. */                                \
[[no_unique_address]] ve::meta::null_type BOOST_PP_CAT(ve_impl_autoinit_, name) =                           \
[&](){                                                                                                      \
    return ve::detail::assignment_helper {                                                                  \
        [&](auto&&... args) {                                                                               \
            get_storage().emplace<hidden_type>(get_id(), std::forward<decltype(args)>(args)...);            \
        }                                                                                                   \
    };                                                                                                      \
}()




// Implementation detail. Call VE_FUNCTION_COMPONENT or VE_CONST_FUNCTION_COMPONENT to use this macro.
#define VE_IMPL_FUNCTION_COMPONENT(name, hidden_name, capture_name, side, const_kw, ...)                    \
/* Macro preceded by return type. Capture it. */                                                            \
capture_name(void);                                                                                         \
                                                                                                            \
/* Create a wrapping function which will either call the default implementation, */                         \
/* or get a function_component from the registry, if it exists. */                                          \
typename ve::meta::mft<decltype(&most_derived_t::capture_name)>::return_type                                \
name(auto&&... args) const_kw {                                                                             \
    using component_type  = ve::named_function_component<#name, side>;                                      \
                                                                                                            \
    if (has_dynamic_behaviour()) [[unlikely]] {                                                             \
        const auto& fn = get_component<component_type>();                                                   \
                                                                                                            \
        return ve::detail::fc_invoke_wrapper<                                                               \
            decltype(&most_derived_t::hidden_name)                                                          \
        >::type::invoke(fn, this, std::forward<decltype(args)>(args)...);                                   \
    } else {                                                                                                \
        return hidden_name(std::forward<decltype(args)>(args)...);                                          \
    }                                                                                                       \
}                                                                                                           \
                                                                                                            \
                                                                                                            \
/* Information struct so entity can deduce if the property is present at compile time. */                   \
template <typename T> requires std::is_same_v<T, ve::named_function_component<#name, side>>                 \
constexpr static auto ve_impl_component_info(void) {                                                        \
    return ve::static_component_fn_info<side> { };                                                          \
}                                                                                                           \
                                                                                                            \
                                                                                                            \
/* Add component to registry on entity construction. */                                                     \
[[no_unique_address]] ve::meta::null_type BOOST_PP_SEQ_CAT((ve_impl_autoinit_)(name)) =                     \
[&](){                                                                                                      \
    using fptr_type = typename ve::meta::mft<                                                               \
        decltype(&most_derived_t::hidden_name)                                                              \
    >::freed_pointer_type;                                                                                  \
                                                                                                            \
    get_storage().emplace<ve::named_function_component<#name, side>>(                                       \
        get_id(),                                                                                           \
        (fptr_type) [](const_kw void* self, ve::universal auto... args) {                                   \
            ((const_kw most_derived_t*) self)->name(std::forward<decltype(args)>(args)...);                 \
        }                                                                                                   \
    );                                                                                                      \
                                                                                                            \
    return ve::meta::null_type { };                                                                         \
}();                                                                                                        \
                                                                                                            \
                                                                                                            \
/* Default implementation. Implementation starts after macro ends. */                                       \
typename ve::meta::mft<decltype(&most_derived_t::capture_name)>::return_type                                \
hidden_name(__VA_ARGS__) const_kw




// Create a static component inside the current class.
// The component will act as a normal class member, but calls the registry to get and set its value.
// If the component type is not already a ve::component, it will be wrapped in a ve::named_value_component.
// If your type requires an initializer, you should provide it after the macro, e.g:
// int VE_COMPONENT(my_int) = 3;
//
// Variadic parameters can be used to provide one or more optional parameters.
// The parameters after name are only used for creating the named_value_component, and are ignored if the component
// is already a ve::component.
// name:       The name of the component.
// side:       The side(s) (client / server) the component exists on. Defaults to server.
// csm:        The serialization mode(s) for the component (binary / string). Defaults to binary.
// serializer: A serializer for the component. Defaults to ve::meta::null_type,
//             indicating that serialization should be handled automatically.
#define VE_COMPONENT(name, ...)                                                                             \
VE_IMPL_COMPONENT(                                                                                          \
    name,                                                                                                   \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_hidden_type)),                                                       \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_return_type)),                                                       \
    ve::side::VE_IMPL_VARIADIC_DEFAULT(0, SERVER __VA_OPT__(,) __VA_ARGS__),                                \
    ve::component_serialization_mode::VE_IMPL_VARIADIC_DEFAULT(1, BINARY __VA_OPT__(,) __VA_ARGS__),        \
    VE_IMPL_VARIADIC_DEFAULT(2, ve::meta::null_type __VA_OPT__(,) __VA_ARGS__)                              \
)


// Create a static function component inside the current class.
// The component will act as a normal member function, but if the value of the component is changed in the registry,
// the updated version of the component will be called instead.
//
// This macro should be used in place of the name and arguments of your function.
// e.g. to create a function component, returning an int and taking two int parameters, you would write the following:
// int VE_FUNCTION_COMPONENT(my_function, SERVER, int x, int y) { return x + y; }
#define VE_FUNCTION_COMPONENT(name, cside, ...)                                                             \
VE_IMPL_FUNCTION_COMPONENT(                                                                                 \
    name,                                                                                                   \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_default)),                                                           \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_rt_capture)),                                                        \
    ve::side::cside,                                                                                        \
    /* Not Const */ __VA_OPT__(,)                                                                           \
    __VA_ARGS__                                                                                             \
)


// Equivalent to VE_FUNCTION_COMPONENT, but creates a const member function.
#define VE_CONST_FUNCTION_COMPONENT(name, cside, ...)                                                       \
VE_IMPL_FUNCTION_COMPONENT(                                                                                 \
    name,                                                                                                   \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_default)),                                                           \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_rt_capture)),                                                        \
    ve::side::cside,                                                                                        \
    const __VA_OPT__(,)                                                                                     \
    __VA_ARGS__                                                                                             \
)


#endif // IDE Check