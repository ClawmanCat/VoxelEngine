#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entity/static_entity.hpp>
#include <VoxelEngine/ecs/component/function_component.hpp>
#include <VoxelEngine/ecs/component/self_component.hpp>
#include <VoxelEngine/utility/traits/null_type.hpp>
#include <VoxelEngine/utility/traits/function_traits.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>


#define ve_impl_this_t std::remove_reference_t<decltype(*this)>


namespace ve::detail {
    // This is an ugly workaround to prevent static properties from being uninitialized (i.e. not in the registry).
    // The trick here is that given the following code:
    // meta::null_type a = property_initializer { this };
    // meta::null_type b = property_initializer { this } = SomeInitializer;
    // "a" gets constructed from the result of operator meta::null_type(void) const &&,
    // but "b" gets constructed using meta::null_type(void) const &.
    // By deleting the former, we can ensure that an initializer is provided for each property.
    // Note that in the case of "b", property_initializer::operator=(SomeInitializer) is also called.
    // This is the point where the actual initializer of the property (Init) is called.
    template <typename T, auto Init, typename B> struct property_initializer {
        B* parent;

        explicit property_initializer(B* parent) : parent(parent) {}

        template <typename Arg> requires std::is_constructible_v<T, Arg>
        property_initializer& operator=(Arg&& arg) {
            // Note: if we're moving out of some other entity, that entity will already have values for the components.
            // Since we will take their ID, make sure to not overwrite those components with empty values.
            if (!(parent->get_flags() & ve::static_entity_flags::move_constructed)) Init(parent, fwd(arg));
            return *this;
        }

        operator meta::null_type(void) const & { return {}; }
        operator meta::null_type(void) const && = delete; // Please initialize the property!
    };
};


// Some IDEs seem to struggle with the macro below, so just pretend it doesn't exist during the IDE pass.
// Note: for parsers like Clangd, you should pass IDE_PASS manually to disable this section.
#if defined(__INTELLISENSE__) || defined(__JETBRAINS_IDE__) || defined(IDE_PASS)
    #define VE_COMPONENT(name) name
    #define VE_COMPONENT_FN(name) name
#else


// Generates a component within the current static entity class.
// Usage: component_type VE_COMPONENT(name) = initializer;
// E.g. transform_component VE_COMPONENT(transform) = {};
#define VE_COMPONENT(name)                                                                          \
/* Capture component type (preceding macro) */                                                      \
static ve_hide(return_type_capture)(void) { VE_UNREACHABLE; }                                       \
using ve_hide(return_type) = decltype(ve_hide(return_type_capture)());                              \
                                                                                                    \
/* Assert we're calling from a class derived from static_entity */                                  \
void ve_hide(base_check)(void) {                                                                    \
    static_assert(                                                                                  \
        std::is_base_of_v<ve::static_entity, std::remove_cvref_t<decltype(*this)>>,                 \
        "Classes that use VE_COMPONENT must derive from ve::static_entity."                         \
    );                                                                                              \
}                                                                                                   \
                                                                                                    \
/* Generate getters & setters */                                                                    \
ve_hide(return_type)& ve_hide(get_##name)(void) {                                                   \
    return get<ve_hide(return_type)>();                                                             \
}                                                                                                   \
                                                                                                    \
const ve_hide(return_type)& ve_hide(get_##name)(void) const {                                       \
    return get<ve_hide(return_type)>();                                                             \
}                                                                                                   \
                                                                                                    \
void ve_hide(set_##name)(const ve_hide(return_type)& value) {                                       \
    set<ve_hide(return_type)>(value);                                                               \
}                                                                                                   \
                                                                                                    \
void ve_hide(set_##name)(ve_hide(return_type)&& value) {                                            \
    set<ve_hide(return_type)>(std::move(value));                                                    \
}                                                                                                   \
                                                                                                    \
                                                                                                    \
/* Property to make it appear the component is a class member. */                                   \
__declspec(property ( get = ve_hide(get_##name), put = ve_hide(set_##name) ))                       \
ve_hide(return_type) name;                                                                          \
                                                                                                    \
                                                                                                    \
/* The actual initializer. */                                                                       \
/* Since there is no other way to force users to initialize a property, use this instead. */        \
[[no_unique_address]] ve::meta::null_type ve_hide(name##_initializer) =                             \
    ve::detail::property_initializer<                                                               \
        ve_hide(return_type),                                                                       \
        [](auto* self, auto&& arg) { self->name = fwd(arg); },                                      \
        ve_impl_this_t                                                                              \
    >(this)


// Generates a function component within the current static entity class.
// Usage: return_type VE_COMPONENT_FN(name)(args...) { body; }
// E.g.:
// ve::nanoseconds age = 0ns;
//
// void VE_COMPONENT_FN(update)(entt::entity self, ve::nanoseconds dt) {
//     this->age += dt; // Yes, "this" may be used in component functions.
// }
//
// Note: component functions may not be templated or use auto in their signature, but may be const.
// (The ECS systems always acts on non-const instances of each entity.)
// Note: when called as a component, the function signature gains a parameter of type const invocation_context& at index 0,
// which is used to provide the this pointer for the called member function.
// Systems that wish to support static function components should set their used component type to include such a parameter.
#define VE_COMPONENT_FN(name)                                                                       \
/* Capture component type (preceding macro) */                                                      \
static ve_hide(return_type_capture)(void) { VE_UNREACHABLE; }                                       \
using ve_hide(return_type) = decltype(ve_hide(return_type_capture)());                              \
                                                                                                    \
                                                                                                    \
/* Assert we're calling from a class derived from static_entity */                                  \
void ve_hide(base_check)(void) {                                                                    \
    static_assert(                                                                                  \
        std::is_base_of_v<ve::static_entity, std::remove_cvref_t<decltype(*this)>>,                 \
        "Classes that use VE_COMPONENT_FN must derive from ve::static_entity."                      \
    );                                                                                              \
}                                                                                                   \
                                                                                                    \
/* Implementation when calling from the entity: use the value in the ECS. */                        \
static ve_hide(return_type) ve_hide(name##_common)(auto self, auto&&... args) {                     \
    using self_type = std::remove_cvref_t<std::remove_pointer_t<decltype(self)>>;                   \
    using fn_traits = ve::meta::function_traits<decltype(&self_type::ve_hide(name##_impl))>;        \
    using real_args = typename fn_traits::arguments;                                                \
                                                                                                    \
    return [&] <typename... Args> (ve::meta::pack<Args...>) {                                       \
        return self->template get<ve::named_function_component<                                     \
            #name,                                                                                  \
            ve_hide(return_type),                                                                   \
            const ve::invocation_context&,                                                          \
            Args...                                                                                 \
        >>()(ve::invocation_context { &self->get_registry(), self->get_id() }, fwd(args)...);       \
    }(real_args { });                                                                               \
}                                                                                                   \
                                                                                                    \
/* Provide const and non const versions. */                                                         \
/* We can't check the constness of the actual function in a static context, */                      \
/* so just use a static assert instead. */                                                          \
template <typename T = void>                                                                        \
ve_hide(return_type) name(auto&&... args) const {                                                   \
    using fn_traits = ve::meta::function_traits<decltype(&ve_impl_this_t::ve_hide(name##_impl))>;   \
                                                                                                    \
    if constexpr (fn_traits::is_const) {                                                            \
        return ve_hide(name##_common)(this, fwd(args)...);                                          \
    } else {                                                                                        \
        static_assert(                                                                              \
            ve::meta::always_false_v<T>,                                                            \
            "Cannot invoke non-const member function on const object."                              \
        );                                                                                          \
    }                                                                                               \
}                                                                                                   \
                                                                                                    \
template <typename = void>                                                                          \
ve_hide(return_type) name(auto&&... args) {                                                         \
    return ve_hide(name##_common)(this, fwd(args)...);                                              \
}                                                                                                   \
                                                                                                    \
                                                                                                    \
/* Add an empty member whose initializer adds the component to the ECS. */                          \
[[no_unique_address]] ve::meta::null_type ve_hide(name##_initializer) = [&] {                       \
    using fn_traits = ve::meta::function_traits<decltype(&ve_impl_this_t::ve_hide(name##_impl))>;   \
    using real_args = typename fn_traits::arguments;                                                \
    using self_type = std::remove_cvref_t<std::remove_pointer_t<decltype(this)>>;                   \
                                                                                                    \
    [&] <typename... Args> (ve::meta::pack<Args...>) {                                              \
        using component_t = ve::named_function_component<                                           \
            #name,                                                                                  \
            ve_hide(return_type),                                                                   \
            const ve::invocation_context&,                                                          \
            Args...                                                                                 \
        >;                                                                                          \
                                                                                                    \
        set<component_t>(component_t {                                                              \
            /* Note: signatures must match exactly, so we can't use perfect forwarding here. */     \
            [](const ve::invocation_context& ctx, Args... args) -> ve_hide(return_type) {           \
                self_type* self = (self_type*) ctx.registry                                         \
                    ->template get_component<ve::self_component>(ctx.entity).self;                  \
                                                                                                    \
                return self->ve_hide(name##_impl)(std::move(args)...);                              \
            }                                                                                       \
        });                                                                                         \
    } (real_args { });                                                                              \
                                                                                                    \
    return ve::meta::null_type { };                                                                 \
}();                                                                                                \
                                                                                                    \
/* Default implementation. Arguments and body come after macro. */                                  \
ve_hide(return_type) ve_hide(name##_impl)


#endif // End of IDE-skipped section.