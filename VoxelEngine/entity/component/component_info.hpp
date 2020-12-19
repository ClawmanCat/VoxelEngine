#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/function_traits.hpp>

#include <ctti/type_id.hpp>


namespace ve::detail {
    template <auto fn>
    struct member_fn_helper {
        using traits = meta::function_traits<decltype(fn)>;
        
        template <typename... Ts> struct unpacked {
            constexpr static auto wrap_member_fn(void) {
                if constexpr (traits::is_const) {
                    return [](const typename traits::class_type* self, Ts... args) { return (self->*fn)(args...); };
                } else {
                    return [](typename traits::class_type* self, Ts... args) { return (self->*fn)(args...); };
                }
            }
        };
        
        constexpr static inline auto value = traits::argument_types::template expand_inside<unpacked>::wrap_member_fn();
    };
}


#define VE_IMPL_DATA_COMPONENT(member, cls)                     \
ve::data_component_info {                                       \
    #member,                                                    \
    ctti::type_id<decltype(member)>(),                          \
    sizeof(member),                                             \
    [](void* self) { return (void*) &(((cls*) self)->member); } \
}


#define VE_IMPL_FN_COMPONENT(method, cls)                       \
ve::function_component_info {                                   \
    #method,                                                    \
                                                                \
    ctti::type_id<                                              \
        typename ve::meta::function_traits<                     \
            decltype(&cls::method)                              \
        >::return_type                                          \
    >(),                                                        \
                                                                \
    ve::meta::function_traits<                                  \
        decltype(&cls::method)                                  \
    >::argument_types::left_fold(                               \
        std::vector<ctti::type_id_t> { },                       \
        [] <typename Pack> (auto&& vec, const Pack& type) {     \
            vec.push_back(                                      \
                ctti::type_id<typename Pack::head>()            \
            );                                                  \
                                                                \
            return std::forward<decltype(vec)>(vec);            \
        }                                                       \
    ),                                                          \
                                                                \
    (void*) ve::detail::member_fn_helper<&cls::method>::value   \
}

// Construct a data_component_info object for the given member variable.
#define VE_FN_COMPONENT(method)                                 \
VE_IMPL_FN_COMPONENT(                                           \
    method,                                                     \
    std::remove_cvref_t<decltype(*this)>                        \
)

// Construct a function_component_info object for the given member function.
#define VE_DATA_COMPONENT(member)                               \
VE_IMPL_DATA_COMPONENT(                                         \
    member,                                                     \
    std::remove_cvref_t<decltype(*this)>                        \
)


#define VE_DATA_COMPONENT_LIST_MACRO(Rep, Data, Elem) ((VE_DATA_COMPONENT(Elem)))

// Given a base class and a set of member variables, overload the method that lists all data components.
#define VE_DATA_COMPONENT_LIST(base, ...)                       \
virtual std::vector<ve::data_component_info>                    \
get_static_component_members(void) const override {             \
    const static auto components = join(                        \
        std::vector<ve::data_component_info> {                  \
            BOOST_PP_SEQ_ENUM(                                  \
                BOOST_PP_SEQ_FOR_EACH(                          \
                    VE_DATA_COMPONENT_LIST_MACRO,               \
                    _,                                          \
                    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)       \
                )                                               \
            )                                                   \
        },                                                      \
        ve::unwrap<void(base)>                                  \
            ::type                                              \
            ::get_static_component_members()                    \
    );                                                          \
                                                                \
    return components;                                          \
}


#define VE_FN_COMPONENT_LIST_MACRO(Rep, Data, Elem) ((VE_FN_COMPONENT(Elem)))

// Given a base class and a set of member functions, overload the method that lists all function components.
#define VE_FN_COMPONENT_LIST(base, ...)                         \
virtual std::vector<ve::function_component_info>                \
get_static_component_methods(void) const override {             \
    const static auto components = join(                        \
        std::vector<ve::function_component_info> {              \
            BOOST_PP_SEQ_ENUM(                                  \
                BOOST_PP_SEQ_FOR_EACH(                          \
                    VE_FN_COMPONENT_LIST_MACRO,                 \
                    _,                                          \
                    BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__)       \
                )                                               \
            )                                                   \
        },                                                      \
        ve::unwrap<void(base)>                                  \
            ::type                                              \
            ::get_static_component_methods()                    \
    );                                                          \
                                                                \
    return components;                                          \
}


namespace ve {
    struct data_component_info {
        std::string_view name;
        ctti::type_id_t type;
        std::size_t size;
        Fn<void*, void*> accessor;
    };
    
    
    struct function_component_info {
        std::string_view name;
        
        ctti::type_id_t return_type;
        std::vector<ctti::type_id_t> arg_types;
        
        void* ptr;
    };
}