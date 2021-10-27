#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/traits/dont_deduce.hpp>
#include <VoxelEngine/utility/traits/pack/pack.hpp>
#include <VoxelEngine/ecs/component/named_component.hpp>


namespace ve {
    class registry;


    template <typename Ret, typename... Args> struct function_component {
        fn<Ret, Args...> value = nullptr;


        function_component(void) = default;
        explicit function_component(fn<Ret, Args...> value) : value(value) {}


        // Note: cannot use perfect forwarding here, Args is always deduced as the exact signature of the function.
        Ret operator()(Args... args) const {
            return value(std::move(args)...);
        }
    };


    // Systems should consider having their associated function component include the invocation context as the first parameter,
    // so static function components can be invoked through said systems as well.
    struct invocation_context {
        registry* registry;
        entt::entity entity;
    };


    template <meta::string_arg Name, typename Ret, typename... Args>
    using named_function_component = named_component<Name, function_component<Ret, Args...>>;


    using update_component = named_function_component<"update", void, const invocation_context&, nanoseconds>;
}