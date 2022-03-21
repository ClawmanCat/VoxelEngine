#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    template <typename From, typename To, auto Cast = [](auto&& f) { return static_cast<To>(f); }>
    class delayed_cast {
    public:
        constexpr explicit delayed_cast(From&& from) : from(fwd(from)) {}
        constexpr explicit delayed_cast(const From& from) : from(from) {}
        
        constexpr operator To(void) const { return std::invoke(Cast, from); }
    private:
        From from;
    };
    
    
    template <typename To, auto Cast = [](auto&& f) { return static_cast<To>(f); }>
    constexpr auto make_delayed_cast(auto&& from) {
        return delayed_cast<std::remove_cvref_t<decltype(from)>, To, Cast> { fwd(from) };
    }


    template <typename To, typename Fn>
    constexpr auto make_delayed_invoker(Fn&& fn) {
        return delayed_cast<Fn, To, [](auto&& fn) { return std::invoke(fn); }> { fwd(fn) };
    }
}