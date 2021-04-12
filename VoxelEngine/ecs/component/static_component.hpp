#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>
#include <VoxelEngine/utility/traits/member_function_traits.hpp>
#include <VoxelEngine/ecs/component/component.hpp>
#include <VoxelEngine/ecs/component/named_component.hpp>
#include <VoxelEngine/ecs/component/function_component.hpp>
#include <VoxelEngine/ecs/component/value_component.hpp>
#include <VoxelEngine/ecs/component/static_component_info.hpp>
#include <VoxelEngine/side/side.hpp>

#include <boost/preprocessor.hpp>


namespace ve::detail {
    // If T is a component, return that components side value, otherwise default to the provided value.
    template <typename T> constexpr side component_side_or(side s) {
        if constexpr (requires { typename T::component_tag; }) return T::side;
        else return s;
    }
    
    
    // If T is a component, return that components CSM, otherwise default to the provided value.
    template <typename T> constexpr component_serialization_mode component_csm_or(component_serialization_mode csm) {
        if constexpr (requires { typename T::component_tag; }) return T::csm;
        else return csm;
    }
    
    
    // If T is a component, returns T, otherwise returns named_value_component<T>.
    template <
        meta::string_arg Name,
        typename T,
        side Side                        = side::SERVER,
        component_serialization_mode CSM = component_serialization_mode::BINARY,
        typename Serializer              = meta::null_type
    > using underlying_component_t =
        std::conditional_t<
            (requires { typename T::component_tag_t; }),
            T,
            named_value_component<Name, T, Side, CSM, Serializer>
        >;
    
    
    // Invokes the provided function component with args,
    // reconstructing the function signature from the provided member function type.
    template <typename MemFn> struct fc_invoke_wrapper {
        using Traits = ve::meta::mft<MemFn>;
        
        template <typename... Args> struct inner {
            constexpr static typename Traits::return_type invoke(auto& cmp, void* self, Args&&... args) {
                cmp.template invoke_unchecked<
                    typename Traits::return_type,
                    void*,
                    Args...
                >(self, std::forward<Args>(args)...);
            }
    
            constexpr static typename Traits::return_type invoke(auto& cmp, const void* self, Args&&... args) {
                cmp.template invoke_unchecked<
                    typename Traits::return_type,
                    const void*,
                    Args...
                >(self, std::forward<Args>(args)...);
            }
        };
        
        using type = typename Traits::argument_types::template expand_inside<inner>;
    };
}


// Used for overloading macros with a variable number of arguments.
#define VE_IMPL_VARIADIC_DEFAULT(Index, Value, ...)                                                         \
BOOST_PP_IF(                                                                                                \
    BOOST_PP_GREATER_EQUAL(                                                                                 \
        Index,                                                                                              \
        BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)                                                                 \
    ),                                                                                                      \
    Value,                                                                                                  \
    BOOST_PP_SEQ_ELEM(                                                                                      \
        BOOST_PP_MIN(                                                                                       \
            Index,                                                                                          \
            BOOST_PP_SUB(                                                                                   \
                BOOST_PP_SEQ_SIZE(                                                                          \
                    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                                   \
                ),                                                                                          \
                1                                                                                           \
            )                                                                                               \
        ),                                                                                                  \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                                               \
    )                                                                                                       \
)


#define VE_IMPL_VARIADIC_REST(Index, ...)                                                                   \
BOOST_PP_REMOVE_PARENS(                                                                                     \
    BOOST_PP_EXPAND(                                                                                        \
        BOOST_PP_IF(                                                                                        \
            BOOST_PP_LESS(                                                                                  \
                BOOST_PP_MIN(                                                                               \
                    Index,                                                                                  \
                    BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)                                                     \
                ),                                                                                          \
                BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)                                                         \
            ),                                                                                              \
            (                                                                                               \
                BOOST_PP_SEQ_ENUM(                                                                          \
                    BOOST_PP_SEQ_REST_N(                                                                    \
                        BOOST_PP_MIN(                                                                       \
                            Index,                                                                          \
                            BOOST_PP_VARIADIC_SIZE(__VA_ARGS__)                                             \
                        ),                                                                                  \
                        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                               \
                    )                                                                                       \
                )                                                                                           \
            ),                                                                                              \
            ()                                                                                              \
        )                                                                                                   \
    )                                                                                                       \
)




// Add a static component to this class.
// If type is a component, the component will remain unwrapped,
// otherwise it will be a named_value_component<type>.
// Variadic argument can be used to initialize the component.
// Type / serializer should be wrapped in parentheses if they contain exposed commas.
#define VE_IMPL_COMPONENT(name, type, side, csm, serializer, ...)                                           \
using ve_impl_hidden_type_##name = ve::detail::underlying_component_t<                                      \
    #name,                                                                                                  \
    VE_UNWRAP(type),                                                                                        \
    side,                                                                                                   \
    csm,                                                                                                    \
    serializer                                                                                              \
>;                                                                                                          \
                                                                                                            \
/* Getters and setters for property */                                                                      \
const VE_UNWRAP(type)& BOOST_PP_SEQ_CAT((ve_impl_)(name)(_getter))(void) const {                            \
    return get_storage().get<ve_impl_hidden_type_##name>(get_id());                                         \
}                                                                                                           \
                                                                                                            \
VE_UNWRAP(type)& BOOST_PP_SEQ_CAT((ve_impl_)(name)(_getter))(void) {                                        \
    return get_storage().get<ve_impl_hidden_type_##name>(get_id());                                         \
}                                                                                                           \
                                                                                                            \
void BOOST_PP_SEQ_CAT((ve_impl_)(name)(_setter))(VE_UNWRAP(type)&& o) {                                     \
    get_storage().replace<ve_impl_hidden_type_##name>(get_id(), std::move(o));                              \
}                                                                                                           \
                                                                                                            \
                                                                                                            \
__declspec(property(                                                                                        \
    get = BOOST_PP_SEQ_CAT((ve_impl_)(name)(_getter)),                                                      \
    put = BOOST_PP_SEQ_CAT((ve_impl_)(name)(_setter))                                                       \
)) VE_UNWRAP(type) name;                                                                                    \
                                                                                                            \
                                                                                                            \
/* Add component to registry on entity construction. */                                                     \
[[no_unique_address]] ve::meta::null_type BOOST_PP_SEQ_CAT((ve_impl_autoinit_)(name)) =                     \
[&](){                                                                                                      \
    get_storage().emplace<ve_impl_hidden_type_##name>(get_id() __VA_OPT__(,) __VA_ARGS__);                  \
    return ve::meta::null_type { };                                                                         \
}();                                                                                                        \
                                                                                                            \
                                                                                                            \
/* Information struct so entity can deduce if the property is present at compile time */                    \
template <typename T> requires std::is_same_v<T, ve_impl_hidden_type_##name>                                \
constexpr static auto ve_impl_component_info(void) {                                                        \
    return ve::static_component_value_info<side, csm> { };                                                  \
}


// Calls above macro with some defaulted arguments.
// Variadic parameters can be used to provide one or more optional parameters.
// name:       The name of the variable.
// type:       The type of the variable. Should be wrapped if it contains any exposed parentheses.
// side:       The side(s) (client / server) the component exists on. Defaults to server.
//             If 'type' is a component, this parameter is ignored in favor of the value from the component.
// csm:        The serialization mode(s) for the component (binary / string). Defaults to binary.
//             If 'type' is a component, this parameter is ignored in favor of the value from the component.
// serializer: A serializer for the component. Defaults to ve::meta::null_type,
//             indicating the component should be serialized automatically.
// arguments:  Constructor arguments for the component. Defaults to empty.
#define VE_COMPONENT(name, type, ...)                                                                       \
VE_IMPL_COMPONENT(                                                                                          \
    name,                                                                                                   \
    type,                                                                                                   \
    ve::detail::component_side_or<VE_UNWRAP(type)>(                                                         \
        ve::side::VE_IMPL_VARIADIC_DEFAULT(0, SERVER, __VA_ARGS__)                                          \
    ),                                                                                                      \
    ve::detail::component_csm_or<VE_UNWRAP(type)>(                                                          \
        ve::component_serialization_mode::VE_IMPL_VARIADIC_DEFAULT(1, BINARY, __VA_ARGS__)                  \
    ),                                                                                                      \
    VE_IMPL_VARIADIC_DEFAULT(2, ve::meta::null_type, __VA_ARGS__),                                          \
    VE_IMPL_VARIADIC_REST(3, __VA_ARGS__)                                                                   \
)


// Same as above, but assumes 'type' is a component type,
// and as such does not accept side, csm and serializer parameters.
#define VE_PROPER_COMPONENT(name, type, ...) \
VE_COMPONENT(name, type, BOTH, BINARY, ve::meta::null_type, __VA_ARGS__)


// Adds a static function component to this class.
// Variadic parameter should contain the arguments of the method.
// Note: this macro should be used for the declaration of the function in question. e.g:
// void VE_FUNCTION_COMPONENT(my_fn, SERVER) { do_thing_a(); do_thing_b(); }
#define VE_IMPL_FUNCTION_COMPONENT(name, hidden_name, capture_name, invoker_name, side, const_kw, ...)      \
/* Macro preceeded by return type. Capture it. */                                                           \
capture_name(void) {}                                                                                       \
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


#define VE_FUNCTION_COMPONENT(name, cside, ...)                                                             \
VE_IMPL_FUNCTION_COMPONENT(                                                                                 \
    name,                                                                                                   \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_default)),                                                           \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_rt_capture)),                                                        \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(invoker)),                                                            \
    ve::side::cside,                                                                                        \
    /* Not Const */ __VA_OPT__(,)                                                                           \
    __VA_ARGS__                                                                                             \
)


#define VE_CONST_FUNCTION_COMPONENT(name, cside, ...)                                                       \
VE_IMPL_FUNCTION_COMPONENT(                                                                                 \
    name,                                                                                                   \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_default)),                                                           \
    BOOST_PP_SEQ_CAT((ve_impl_)(name)(_rt_capture)),                                                        \
    ve::side::cside,                                                                                        \
    const __VA_OPT__(,)                                                                                     \
    __VA_ARGS__                                                                                             \
)