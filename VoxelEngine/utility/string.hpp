#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/is_type.hpp>
#include <VoxelEngine/utility/functional.hpp>

#include <boost/pfr.hpp>
#include <ctti/type_id.hpp>

#include <sstream>


namespace ve {
    constexpr inline auto char_tolower = [](char ch) { return (char) std::tolower((unsigned char) ch); };
    constexpr inline auto char_toupper = [](char ch) { return (char) std::toupper((unsigned char) ch); };
    
    
    template <typename T> requires std::is_unsigned_v<T>
    inline std::string to_hex_string(T value, bool prepend_zero_x = true, std::size_t length = sizeof(T) << 1) {
        constexpr char chars[] = "0123456789ABCDEF";
        
        std::size_t offset = 2 * prepend_zero_x;
        std::string result(length + offset, '0');
        
        for (std::size_t i = 0, j = 4 * (length - 1); i < length; i += 1, j -= 4) {
            result[offset + i] = chars[(value >> j) & 0x0F];
        }
        
        if (prepend_zero_x) result[1] = 'x';
        return result;
    }
    
    
    template <typename T, bool CalledInternally = false>
    inline std::string to_string(const T& value) {
        // If T can be converted to a string, convert it.
        if      constexpr (requires (T t) { std::to_string(t); }) return std::to_string(value);
        else if constexpr (requires (T t) { (std::string) t;   }) return (std::string) value;
        else if constexpr (requires (T t) { std::string { t }; }) return std::string { value };
        else if constexpr (requires (T t) { t.to_string();     }) return value.to_string();
        
        // If T is an exception, return the exception message.
        else if constexpr (std::is_base_of_v<std::exception, T>) {
            return value.what();
        }
        
        // If T is streamable, stream it, then convert the stream to a string.
        else if constexpr (requires (T t, std::stringstream s) { s << t; }) {
            std::stringstream s;
            s << value;
            return s.str();
        }
    
        // If T is trivial, print the values of its fields.
        else if constexpr (requires { boost::pfr::tuple_size_v<T>; }) {
            std::stringstream s;
            s << boost::pfr::io(value);
            return s.str();
        }
        
        // If T is a pointer, print its type and address and the string representation of the contained value.
        else if constexpr (std::is_pointer_v<T>) {
            using deref_t = std::remove_pointer_t<T>;
            
            auto object_string  = to_string<deref_t, true>(*value);
            auto type_string    = ctti::nameof<deref_t>;
            auto address_string = to_hex_string((std::size_t) value);
            
            std::string result = "[Pointer to "s + type_string + " @ " + address_string + "]";
            if (!object_string.empty()) result += (": "s + object_string);
            
            return result;
        }
        
        // Fallback option: print the type and address of the object.
        // Don't print anything if we're being called from the pointer string conversion,
        // as that will already print this info.
        else {
            if constexpr (CalledInternally) return "";
            
            else {
                auto type_string    = ctti::nameof<T>;
                auto address_string = to_hex_string((std::size_t) std::addressof(value));
            
                return "["s + type_string + " @ " + address_string + "]";
            }
        }
    }
    
    
    template <typename... Ts> inline std::string cat(const Ts&... objects) {
        return (to_string(objects) + ...);
    }


    template <typename... Ts> inline std::string cat_with(const std::string& separator, const Ts&... objects) {
        auto result = ([&](auto&& arg) { return to_string(arg) + separator; }(objects) + ...);
        result.resize(result.length() - separator.length());
        
        return result;
    }


    template <ranges::range Rng>
    inline std::string cat_range(const Rng& range) {
        std::string result;
        for (const auto& str : range | views::transform(ve_wrap_callable(to_string))) result += str;
        return result;
    }


    template <ranges::range Rng>
    inline std::string cat_range_with(const Rng& range, const std::string& separator) {
        std::string result;
        for (const auto& str : range | views::transform(ve_wrap_callable(to_string)) | views::intersperse(separator)) result += str;
        return result;
    }
    
    
    inline std::string replace_substring(std::string_view source, std::string_view find, std::string_view replace_with) {
        std::string result;
        
        while (source.length() > 0) {
            if (source.starts_with(find)) {
                result += replace_with;
                source.remove_prefix(find.length());
            } else {
                result += source[0];
                source.remove_prefix(1);
            }
        }
        
        return result;
    }
    
    
    inline std::string to_sentence_case(std::string_view s) {
        std::string result { s };
        
        if (!result.empty()) {
            result[0] = char_toupper(result[0]);
            for (auto& ch : result | views::drop(1)) ch = char_tolower(ch);
        }
        
        return result;
    }
    
    
    inline std::string to_lowercase(std::string_view s) {
        return s | views::transform(char_tolower) | ranges::to<std::string>;
    }
    
    
    inline std::string to_uppercase(std::string_view s) {
        return s | views::transform(char_toupper) | ranges::to<std::string>;
    }
}