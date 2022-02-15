#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/traits/is_type.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>

#include <boost/pfr.hpp>
#include <ctti/type_id.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <VoxelEngine/ecs/entt_include.hpp>

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


    inline std::string to_hex_string(std::span<const u8> value, bool prepend_zero_x = true) {
        constexpr char chars[] = "0123456789ABCDEF";

        std::size_t offset = 2 * prepend_zero_x;
        std::string result(2 * value.size() + offset, '0');

        for (std::size_t i = 0; i < value.size(); ++i) {
            result[2 * i + 0 + offset] = chars[(value[i] & 0xF0) >> 4];
            result[2 * i + 1 + offset] = chars[(value[i] & 0x0F) >> 0];
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
        else if constexpr (requires (T t) { t.string();        }) return value.string();
        else if constexpr (requires (T t) { t.cppstring();     }) return value.cppstring();

        // If T is an entity, print its ID.
        else if constexpr (std::is_same_v<T, entt::entity>) {
            return "[Entity "s + to_string(entt::to_integral(value)) + "]";
        }

        // If T is a GLM vector print its elements.
        else if constexpr (meta::glm_traits<T>::is_vector) {
            std::string result = "["s + std::to_string(value[0]);

            for (std::size_t i = 1; i < meta::glm_traits<T>::num_rows; ++i) {
                result += ", ";
                result += std::to_string(value[i]);
            }

            result += "]";
            return result;
        }

        // If T is a GLM vector print its rows.
        else if constexpr (meta::glm_traits<T>::is_matrix) {
            std::string result = "["s;

            for (std::size_t i = 0; i < meta::glm_traits<T>::num_rows; ++i) {
                if (i > 0) result += ", ";
                result += to_string(glm::column(value, i));
            }

            result += "]";
            return result;
        }

        // If T is bool, print true or false.
        else if constexpr (std::is_same_v<T, bool>) {
            return value ? "true" : "false";
        }

        // If T is an exception, return the exception message.
        else if constexpr (std::is_base_of_v<std::exception, T>) {
            return value.what();
        }

        // If T is a container, concatenate each element.
        else if constexpr (requires (T t) { std::cbegin(t), std::cend(t); }) {
            std::string result = "[";

            for (const auto& [i, elem] : value | views::enumerate) {
                if (i > 0) result += ", ";
                result += to_string(elem);
            }

            result += "]";
            return result;
        }
        
        // If T is streamable, stream it, then convert the stream to a string.
        else if constexpr (!std::is_pointer_v<T> && requires (T t, std::stringstream s) { s << t; }) {
            std::stringstream s;
            s << value;
            return s.str();
        }
    
        // If T is trivial, print the values of its fields.
        // TODO: Replace with decomposer.
        else if constexpr (std::is_aggregate_v<T>) {
            std::string result = "[";

            boost::pfr::for_each_field(value, [&, i = 0] (const auto& f) mutable {
                if (i++ > 0) result += ", ";
                result += to_string(f);
            });

            result += "]";
            return result;
        }
        
        // If T is a pointer, print its type and address and the string representation of the contained value.
        else if constexpr (std::is_pointer_v<T>) {
            using deref_t = std::remove_cvref_t<std::remove_pointer_t<T>>;
            
            auto object_string  = value ? to_string<deref_t, true>(*value) : "NULL";
            auto type_string    = ctti::nameof<deref_t>().cppstring();
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
                auto type_string    = ctti::nameof<T>().cppstring();
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