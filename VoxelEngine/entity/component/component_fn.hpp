#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/container_utils.hpp>
#include <VoxelEngine/utils/meta/traits/function_traits.hpp>
#include <VoxelEngine/utils/meta/traits/null_type.hpp>
#include <VoxelEngine/entity/component/named_component.hpp>
#include <VoxelEngine/entity/component/script_component.hpp>
#include <VoxelEngine/entity/component/required_component_registry.hpp>

#include <ctti/type_id.hpp>
#include <boost/preprocessor.hpp>


namespace ve::detail {
    // Gives the function pointer type for the given return type and argument pack.
    template <typename Ret, typename ArgPack> class signature_to_ptr {
        template <typename... Args> using ret_type_bound = Fn<Ret, Args...>;
    public:
        using type = typename ArgPack::template expand_inside<ret_type_bound>;
    };
    
    
    // Gives a vector of ctti type IDs for the given arguments.
    template <typename... Args> struct args_to_ctti_vector {
        const static inline std::vector<ctti::type_id_t> value { ctti::type_id<Args>()... };
    };
}


#define VE_IMPL_STATIC_COMPONENT_FN(name, ret_type, arg_types, ...)                         \
{                                                                                           \
    using component_t = ve::named_component<ve::script_component, #name>;                   \
                                                                                            \
    if (is_dynamic() && owner->has<component_t>(id)) [[unlikely]] {                         \
        auto& script = owner->get<component_t>(id).value;                                   \
                                                                                            \
        using fn_type = typename ve::detail::signature_to_ptr<                              \
            ret_type,                                                                       \
            decltype(this),                                                                 \
            arg_types                                                                       \
        >::type;                                                                            \
                                                                                            \
        return static_cast<fn_type>(script.fn_ptr)(this, __VA_ARGS__);                      \
    }                                                                                       \
}


// Makes the current function replaceable at runtime.
// Component functions should not be templated, or use auto in their signature.
// Usage:
// return_type my_fn(int x, int y, bool z) {
//     VE_STATIC_COMPONENT_FN(my_fn, x, y, z);
//     < normal code goes here >
// }
#define VE_STATIC_COMPONENT_FN(name, ...)                                                   \
VE_IMPL_STATIC_COMPONENT_FN(                                                                \
    name,                                                                                   \
                                                                                            \
    ve::meta::function_traits<                                                              \
        decltype(&std::remove_cvref_t<decltype(*this)>::name)                               \
    >::return_type,                                                                         \
                                                                                            \
    ve::meta::function_traits<                                                              \
        decltype(&std::remove_cvref_t<decltype(*this)>::name)                               \
    >::argument_types,                                                                      \
                                                                                            \
    __VA_ARGS__                                                                             \
)


// Transforms ((SomeType, name), ((SomeType<x, y>), name), ...) to (SomeType, SomeType<x, y>, ...)
#define VE_IMPL_VARARGS_TO_K_MACRO(Rep, Data, Elem)                                         \
( typename ve::unwrap<void(BOOST_PP_TUPLE_ELEM(0, Elem))>::type )

#define VE_IMPL_VARARGS_TO_K(...)                                                           \
BOOST_PP_SEQ_ENUM(                                                                          \
    BOOST_PP_SEQ_FOR_EACH(                                                                  \
        VE_IMPL_VARARGS_TO_K_MACRO,                                                         \
        _,                                                                                  \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                               \
    )                                                                                       \
)


// Transforms ((SomeType, name), ((SomeType<x, y>), name), ...) to (name, name, ...)
#define VE_IMPL_VARARGS_TO_V_MACRO(Rep, Data, Elem)                                         \
( BOOST_PP_TUPLE_ELEM(1, Elem) )

#define VE_IMPL_VARARGS_TO_V(...)                                                           \
BOOST_PP_SEQ_ENUM(                                                                          \
    BOOST_PP_SEQ_FOR_EACH(                                                                  \
        VE_IMPL_VARARGS_TO_V_MACRO,                                                         \
        _,                                                                                  \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                               \
    )                                                                                       \
)


// Transforms ((SomeType, name), ((SomeType<x, y>), name), ...) to (SomeType name, SomeType<x, y> name, ...)
#define VE_IMPL_VARARGS_TO_KV_MACRO(Rep, Data, Elem)                                        \
(                                                                                           \
    typename ve::unwrap<void(BOOST_PP_TUPLE_ELEM(0, Elem))>::type                           \
    BOOST_PP_TUPLE_ELEM(1, Elem)                                                            \
)

#define VE_IMPL_VARARGS_TO_KV(...)                                                          \
BOOST_PP_SEQ_ENUM(                                                                          \
    BOOST_PP_SEQ_FOR_EACH(                                                                  \
        VE_IMPL_VARARGS_TO_KV_MACRO,                                                        \
        _,                                                                                  \
        BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)                                               \
    )                                                                                       \
)


// Makes the current function a runtime component.
// This is different from VE_STATIC_COMPONENT_FN, because components created with that macro
// still use local storage by default, unlike VE_COMPONENT_FN, which always results in a
// runtime component. The component is still nonremovable however.
// This allows for the generation of views over components that are often iterated over,
// but not required for all entities.
// If return_type or argument types contain commas, they should be wrapped in parentheses.
// Const and non-const versions of this macro are provided.
// Usage:
// VE_DYNAMIC_COMPONENT_FN(SomeClass, MyFunction, SomeType, (SomeType, Argname), ((SomeType<x, y>), Argname), ...) { ... }
#define VE_IMPL_DYNAMIC_COMPONENT_FN(const_kw, cls, name, return_type, ...)                 \
                                                                                            \
/* Marks the component as nonremovable at startup. */                                       \
const static inline ve::meta::null_type BOOST_PP_CAT(ve_impl_regvar_01_, __LINE__) = [](){  \
    ve::required_component_registry::instance().add(                                        \
        ctti::type_id<cls>(),                                                               \
        ctti::type_id<ve::named_component<ve::script_component, #name>>()                   \
    );                                                                                      \
                                                                                            \
    return ve::meta::null_type { };                                                         \
}();                                                                                        \
                                                                                            \
/* Adds the component to the registry on object construction. */                            \
[[no_unique_address]] ve::meta::null_type BOOST_PP_CAT(ve_impl_regvar_02_, __LINE__) =      \
[this](){                                                                                   \
    owner->emplace<ve::named_component<ve::script_component, #name>>(                       \
        id,                                                                                 \
        ve::script_component {                                                              \
            BOOST_PP_STRINGIZE(cls::name),                                                  \
            (Fn<                                                                            \
                ve::unwrap<void(return_type)>::type,                                        \
                const_kw cls*,                                                              \
                VE_IMPL_VARARGS_TO_K(__VA_ARGS__)                                           \
            >) [](const_kw cls* self, VE_IMPL_VARARGS_TO_KV(__VA_ARGS__)) {                 \
                return self->BOOST_PP_SEQ_CAT((ve_impl_)(name)(_default_impl))              \
                    (VE_IMPL_VARARGS_TO_V(__VA_ARGS__));                                    \
            }                                                                               \
        }                                                                                   \
    );                                                                                      \
                                                                                            \
    return ve::meta::null_type { };                                                         \
}();                                                                                        \
                                                                                            \
/* Wraps the component method. */                                                           \
ve::unwrap<void(return_type)>::type name(VE_IMPL_VARARGS_TO_KV(__VA_ARGS__)) const_kw {     \
    auto& script = owner                                                                    \
        ->get<ve::named_component<ve::script_component, #name>>(id)                         \
        .value;                                                                             \
                                                                                            \
    return static_cast<                                                                     \
        ve::Fn<                                                                             \
            ve::unwrap<void(return_type)>::type,                                            \
            const_kw cls*,                                                                  \
            VE_IMPL_VARARGS_TO_K(__VA_ARGS__)                                               \
        >                                                                                   \
    >(script.fn_ptr)(this, VE_IMPL_VARARGS_TO_V(__VA_ARGS__));                              \
}                                                                                           \
                                                                                            \
/* Signature for default implementation. */                                                 \
ve::unwrap<void(return_type)>::type                                                         \
BOOST_PP_SEQ_CAT((ve_impl_)(name)(_default_impl))                                           \
(VE_IMPL_VARARGS_TO_KV(__VA_ARGS__)) const_kw


#define VE_DYNAMIC_COMPONENT_FN(cls, name, return_type, ...) \
VE_IMPL_DYNAMIC_COMPONENT_FN(, cls, name, return_type, __VA_ARGS__)

#define VE_CONST_DYNAMIC_COMPONENT_FN(cls, name, return_type, ...) \
VE_IMPL_DYNAMIC_COMPONENT_FN(const, cls, name, return_type, __VA_ARGS__)