#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/entt_include.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/traits/is_type.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>

#include <boost/pfr.hpp>
#include <ctti/type_id.hpp>
#include <glm/gtc/matrix_access.hpp>

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


    // Splits the provided string into multiple lines by inserting \n characters, assuring no line is longer than max_length.
    // The string can only be split after characters in the separator list.
    // If there is no character from the separator list to split the string on, the string is split at exactly max_length.
    inline std::string limit_line_length(const std::string& src, std::size_t max_length, const std::string& separators = " \n") {
        // Finds the last separator in the range [current, limit) or returns limit if no separator exists.
        auto find_next_sep = [&] (std::size_t current, std::size_t limit) -> std::size_t {
            for (std::size_t i = limit; i > current && i < src.size(); --i) {
                if (separators.find(src[i]) != std::string::npos) return i + 1;
            }

            return std::min(limit, src.size());
        };


        std::string result;
        result.reserve(src.length() + (src.length() / max_length) + 1);


        std::size_t current = 0;
        while (current < src.size()) {
            std::size_t next = find_next_sep(current, current + max_length);

            result.append(src.begin() + current, src.begin() + next);
            if (next < src.size()) result.push_back('\n');

            current = next;
        }


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


    inline std::vector<std::string> split(std::string_view source, std::string_view delimiter) {
        std::vector<std::string> result;

        std::size_t pos = std::string_view::npos;
        while (pos = source.find(delimiter), pos != std::string_view::npos) {
            pos += delimiter.length();

            result.emplace_back(source.begin(), source.begin() + pos);
            source.remove_prefix(pos);
        }

        if (!result.empty()) result.emplace_back(source);
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


    inline auto make_indenter(std::ostream& target) {
        return [&target, indent = 0] (i32 delta = 0, bool print_now = true) mutable {
            indent += delta;
            if (print_now) target << std::string((std::size_t) indent, ' ');
        };
    }


    template <typename Indenter>
    inline auto make_scoped_kv_formatter(std::string_view key, std::ostream& stream, Indenter& indenter) {
        struct formatter {
            std::ostream* stream_ptr;
            Indenter* indenter_ptr;

            formatter(std::ostream* stream_ptr, Indenter* indenter_ptr, std::string_view key) : stream_ptr(stream_ptr), indenter_ptr(indenter_ptr) {
                (*indenter_ptr)();
                (*stream_ptr) << "[" << key << "] {\n";
                (*indenter_ptr)(+4, false);
            }

            ~formatter(void) {
                (*indenter_ptr)(-4);
                (*stream_ptr) << "}\n";
            }
        };

        return formatter { &stream, &indenter, key };
    }


    inline std::string escape_chars(std::string_view str, std::string_view chars) {
        std::string result;
        result.reserve(str.size());

        for (char ch : str) {
            if (ranges::contains(chars, ch)) {
                result += '\\';
            }

            result += ch;
        }

        return result;
    }


    inline std::string unescape_chars(std::string_view str, std::string_view chars) {
        std::string result;
        result.reserve(str.size());

        for (auto [i, ch] : str | views::enumerate) {
            if (ch == '\\' && str.size() > i + 1 && ranges::contains(chars, str[i + 1])) {
                continue;
            }

            result += ch;
        }

        return result;
    }
}