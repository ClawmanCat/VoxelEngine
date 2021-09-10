#pragma once

#include <VoxelEngine/core/core.hpp>

#include <variant>


namespace ve {
    template <typename T, typename Error = std::exception>
    class expected {
    public:
        constexpr expected(void) = default;
        
        constexpr expected(T&& value) : value(fwd(value)) {}
        constexpr expected(const T& value) : value(value) {}
        
        constexpr expected(Error&& error) : value(fwd(error)) {}
        constexpr expected(const Error& error) : value(error) {}
        
        
        ve_dereference_as(std::get<T>(value));
        
        constexpr const Error& get_error(void) const { return std::get<Error>(value); }
        constexpr operator bool(void) const { return std::holds_alternative<T>(value); }
    private:
        std::variant<T, Error> value;
    };
    
    
    template <typename Error>
    class expected<void, Error> {
    public:
        constexpr expected(void) = default;
    
        constexpr expected(Error&& error) : value(fwd(error)) {}
        constexpr expected(const Error& error) : value(error) {}
    
        constexpr const Error& get_error(void) const { return *value; }
        constexpr operator bool(void) const { return !value.has_value(); }
    private:
        std::optional<Error> value = std::nullopt;
    };
    
    
    template <typename Fn, typename... Args>
    constexpr expected<std::invoke_result_t<Fn, Args...>> try_call(Fn&& fn, Args&&... args) {
        try {
            if constexpr (std::is_same_v<std::invoke_result_t<Fn, Args...>, void>) {
                std::invoke(fwd(fn), fwd(args)...);
                return expected<void>{};
            } else return std::invoke(fwd(fn), fwd(args)...);
        } catch (const std::exception& e) {
            return e;
        }
    }
    
    
    template <typename T, typename... Args>
    constexpr expected<T> try_make(Args&&... args) {
        try {
            return T(fwd(args)...);
        } catch (const std::exception& e) {
            return e;
        }
    }
}