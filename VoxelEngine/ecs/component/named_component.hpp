#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>


namespace ve {
    namespace detail {
        template <typename T> struct fundamental_type_wrapper {
            T value;

            ve_field_comparable(fundamental_type_wrapper, value);
            ve_arithmetic_as(fundamental_type_wrapper, value);
            ve_bitwise_as(fundamental_type_wrapper, value);

            constexpr operator T(void) const { return value; }
        };


        template <typename T> using named_component_base = std::conditional_t<
            std::is_fundamental_v<T>,
            fundamental_type_wrapper<T>,
            T
        >;
    }


    template <meta::string_arg Name, typename T>
    struct named_component : public detail::named_component_base<T> {
        constexpr static std::string_view name = Name.c_string;


        named_component(void) = default;
        using T::T;


        named_component(const named_component&) = default;
        named_component(named_component&&) = default;

        named_component& operator=(const named_component&) = default;
        named_component& operator=(named_component&&) = default;


        template <typename... Args>
        explicit named_component(Args&&... args) : T(fwd(args)...) {}


        named_component& operator=(const T& o) {
            T::operator=(o);
            return *this;
        }

        named_component& operator=(T&& o) {
            T::operator=(std::move(o));
            return *this;
        }
    };
}