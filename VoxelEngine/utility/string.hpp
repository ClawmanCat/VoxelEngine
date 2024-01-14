#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/container/raii.hpp>
#include <VoxelEngine/utility/meta/is_template.hpp>
#include <VoxelEngine/utility/decompose/decompose.hpp>

#include <range/v3/all.hpp>

#include <string>
#include <string_view>
#include <cctype>
#include <concepts>
#include <optional>
#include <memory>
#include <variant>
#include <sstream>
#include <format>


namespace ve {
    // Forward declarations of all methods in this file.
    template <typename T, bool PIF = true> [[nodiscard]] inline std::string to_string(const T& value);
    [[nodiscard]] inline std::string cat(const auto&... values);
    [[nodiscard]] inline std::string cat_with(const auto& separator, const auto&... values);
    [[nodiscard]] inline std::string cat_range(const auto& range);
    [[nodiscard]] inline std::string cat_range_with(const auto& separator, const auto& range);
    [[nodiscard]] inline std::string to_uppercase(std::string_view src);
    [[nodiscard]] inline std::string to_lowercase(std::string_view src);
    template <std::unsigned_integral T> [[nodiscard]] inline std::string to_hex(T value, std::size_t width = 2 * sizeof(T));
    template <std::integral T> [[nodiscard]] inline std::string thousand_separate(T value, std::string_view separator = "'");




    namespace string_conversion_functions {
        template <glm_vector T> inline std::string from_glm_vector(const T& value) {
            return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
                return "["s + cat_with(", ", value[Is]...) + "]"s;
            } (std::make_index_sequence<glm_traits<T>::size>());
        }


        template <glm_matrix T> inline std::string from_glm_matrix(const T& value) {
            return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
                return "["s + cat_with(", ", value[Is]...) + "]"s;
            } (std::make_index_sequence<glm_traits<T>::width>());
        }


        template <glm_quaternion T> inline std::string from_glm_quat(const T& value) {
            return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
                return "["s + cat_with(", ", value[Is]...) + "]"s;
            } (std::make_index_sequence<4>());
        }


        template <std::derived_from<std::exception> T> inline std::string from_exception(const T& value) {
            return std::format("[Exception {}] {{ {} }}", typename_of<T>(), value.what());
        }


        template <typename T> requires requires (T value) { std::cbegin(value), std::cend(value); }
        inline std::string from_container(const T& value) {
            return views::concat(
                views::single('['),
                value
                    | views::transform([] (const auto& e) { return ve::to_string(e); })
                    | views::intersperse(", ")
                    | views::join,
                views::single(']')
            ) | ranges::to<std::string>;
        }


        // TODO: Print pointed-to type as well.
        template <typename T> requires (
            meta::is_template_v<std::optional, T>   ||
            meta::is_template_v<std::unique_ptr, T> ||
            meta::is_template_v<std::shared_ptr, T> ||
            std::is_pointer_v<T>
        ) inline std::string from_pointer(const T& value) {
            const std::string pointed_value = bool(value) ? to_string(*value) : "NULL"s;
            const std::string address       = to_hex(bool(value) ? (std::uintptr_t) std::addressof(*value) : 0);

            std::string_view pointer_name;
            if constexpr (meta::is_template_v<std::optional, T>)   pointer_name = "Optional";
            if constexpr (meta::is_template_v<std::unique_ptr, T>) pointer_name = "Unique Pointer";
            if constexpr (meta::is_template_v<std::shared_ptr, T>) pointer_name = "Shared Pointer";
            if constexpr (std::is_pointer_v<T>)                    pointer_name = "Raw Pointer";

            return std::format("[{} @ 0x{}] {{ {} }}", pointer_name, address, pointed_value);
        }


        template <typename T> requires meta::is_template_v<std::variant, T>
        inline std::string from_variant(const T& value) {
            auto visitor = [] <typename V> (const V& v) {
                return std::format("[{} holding {}] {{ {} }}", typename_of<T>(), typename_of<V>(), to_string(v));
            };

            return std::visit(visitor, value);
        }


        template <typename T> requires requires (std::stringstream ss, T value) { ss << value; }
        inline std::string from_streamable(const T& value) {
            std::stringstream stream;
            stream << value;

            return stream.str();
        }


        template <decomposable T> inline std::string from_decomposable(const T& value) {
            const auto members = decomposer_for_t<T>::reference_as_tuple(value);

            return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
                std::string_view display_name = typename_of<T>();
                if constexpr (meta::is_template_v<std::pair, T>)  display_name = "Pair";
                if constexpr (meta::is_template_v<std::tuple, T>) display_name = "Tuple";

                return std::format(
                    "[{}] {{ {} }}",
                    display_name,
                    cat_with(", ", to_string(std::get<Is>(members))...)
                );
            } (std::make_index_sequence<std::tuple_size_v<decltype(members)>>());
        }


        template <typename T> inline std::string from_unconvertable(const T& value) {
            return std::format("[{} @ 0x{}]", typename_of<T>(), to_hex((std::uintptr_t) std::addressof(value)));
        }
    }


    /**
     * Converts an object to a string using the first applicable converter in the string_conversion_functions namespace.
     * In order, this method will attempt to perform the following conversions:
     *  1. Direct conversion: objects that can be cast to a string will be cast to a string.
     *  2. Member function: if one of the following member functions exists, it will be invoked (in the specified order):
     *      to_string(), to_str(), to_cppstring(), string(), str(), cppstring()
     *  3. Boolean: Booleans are converted to one of the string constants "true" or "false".
     *  4. GLM Types: Vectors and quaternions are treated as containers (see below), matrices are treated as containers of vectors.
     *  5. Exception Objects: Objects inheriting from std::exception are converted as [Exception <type>] { <e.what()> }.
     *  6. Containers: Containers are printed by concatenating their elements into a comma-separated []-enclosed list.
     *  7. Pointers: Pointers (both raw and smart) and optionals are converted as [<Pointer Type> \@ <Address>] { <to_string(*pointer) | NULL> }
     *  8. Numeric Conversion: objects that can be converted using std::to_string will be converted in this way.
     *  9. Variants: Variant types are converted as [<Variant Type> holding <Active Type>] { <to_string(active_type)> }
     *  10. Streaming: Objects that can be streamed into a std::stringstream are streamed into one, before converting the stream to a string.
     *  11. Decomposable Objects: Objects that can be decomposed into their elements are converted as [<Object Type>] { <to_string(members)...> }
     *  12. Fallback: Print the address and type of the object as [<Object Type> \@ <Address>].
     * @tparam PreventInfiniteRecursion If set to true, any object which invokes to_string on its own type again will be printed as ... the second time to prevent infinite recursion.
     * @param value An object to convert to a string.
     * @return A string representing the provided objects.
     */
    template <typename T, bool PreventInfiniteRecursion> [[nodiscard]] inline std::string to_string(const T& value) {
        namespace scf = string_conversion_functions;


        static thread_local bool invoked = false;
        on_scope_exit on_exit { [&] { invoked = false; } };

        if constexpr (PreventInfiniteRecursion) {
            if (std::exchange(invoked, true)) [[unlikely]] return "...";
        }


        if constexpr      (requires { (std::string) value;           }) return (std::string) value;
        else if constexpr (requires { value.to_string();             }) return value.to_string();
        else if constexpr (requires { value.to_str();                }) return value.to_str();
        else if constexpr (requires { value.to_cppstring();          }) return value.to_cppstring();
        else if constexpr (requires { value.string();                }) return value.string();
        else if constexpr (requires { value.str();                   }) return value.str();
        else if constexpr (requires { value.cppstring();             }) return value.cppstring();
        else if constexpr (std::is_same_v<T, bool>                    ) return value ? "true" : "false";
        else if constexpr (requires { scf::from_glm_vector(value);   }) return scf::from_glm_vector(value);
        else if constexpr (requires { scf::from_glm_matrix(value);   }) return scf::from_glm_matrix(value);
        else if constexpr (requires { scf::from_glm_quat(value);     }) return scf::from_glm_quat(value);
        else if constexpr (requires { scf::from_exception(value);    }) return scf::from_exception(value);
        else if constexpr (requires { scf::from_container(value);    }) return scf::from_container(value);
        else if constexpr (requires { scf::from_pointer(value);      }) return scf::from_pointer(value);
        else if constexpr (requires { std::to_string(value);         }) return std::to_string(value);
        else if constexpr (requires { scf::from_variant(value);      }) return scf::from_variant(value);
        else if constexpr (requires { scf::from_streamable(value);   }) return scf::from_streamable(value);
        else if constexpr (requires { scf::from_decomposable(value); }) return scf::from_decomposable(value);
        else return scf::from_unconvertable(value);
    }


    /**
     * Converts all elements of 'values' to strings using @ref to_string then concatenates them into a single string.
     * @param values The values to convert and concatenate.
     * @return A concatenated string containing the string representations of 'values'.
     */
    [[nodiscard]] inline std::string cat(const auto&... values) {
        return (to_string(values) + ... + "");
    }


    /**
     * Equivalent to @ref cat but a separator is added between each pair of elements.
     * @param separator The separator to insert between the provided values.
     * @param values The values to convert and concatenate.
     * @return A concatenated string containing the string representations of 'values' separated by 'separator'.
     */
    [[nodiscard]] inline std::string cat_with(const auto& separator, const auto&... values) {
        std::string separator_string = to_string(separator);

        std::string joined_string = ([&] (const auto& v) { return to_string(v) + separator; }(values) + ... + "");
        joined_string.resize(joined_string.size() - bool(sizeof...(values)) * separator_string.length());

        return joined_string;
    }


    /**
     * Converts all elements of 'range' to strings using @ref to_string then concatenates them into a single string.
     * @param range A range of values to convert and concatenate.
     * @return A concatenated string containing the string representations of the elements of 'range'.
     */
    [[nodiscard]] inline std::string cat_range(const auto& range) {
        return range
            | views::transform([&] (const auto& v) { return to_string(v); })
            | views::join
            | ranges::to<std::string>;
    }


    /**
     * Equivalent to @ref cat_range but a separator is added between each pair of elements.
     * @param separator The separator to insert between the provided values.
     * @param range A range of values to convert and concatenate.
     * @return A concatenated string containing the string representations of the elements of 'range' separated by 'separator'.
     */
    [[nodiscard]] inline std::string cat_range_with(const auto& separator, const auto& range) {
        const std::string sep_string = to_string(separator);

        return range
           | views::transform([] (const auto& v) { return to_string(v); })
           | views::intersperse(sep_string)
           | views::join
           | ranges::to<std::string>;
    }


    /**
     * Converts the provided string to uppercase.
     * @param src A string to convert.
     * @return The provided string with all lowercase letters replaced with their uppercase variants.
     */
    [[nodiscard]] inline std::string to_uppercase(std::string_view src) {
        return src | views::transform((fn<int, int>) std::toupper) | ranges::to<std::string>;
    }


    /**
     * Converts the provided string to lowercase.
     * @param src A string to convert.
     * @return The provided string with all uppercase letters replaced with their lowercase variants.
     */
    [[nodiscard]] inline std::string to_lowercase(std::string_view src) {
        return src | views::transform((fn<int, int>) std::tolower) | ranges::to<std::string>;
    }


    /**
     * Converts a number to an uppercase string containing its hexadecimal representation.
     * @param value The number to convert.
     * @param width The number of characters in the resulting string.
     *  Using more characters than required for the provided number will pad the string with zeroes.
     *  Using less characters than required for the provided number will cut off the higher-significant digits of the number.
     * @return An uppercase hexadecimal representation of the provided string.
     */
    template <std::unsigned_integral T> [[nodiscard]] inline std::string to_hex(T value, std::size_t width) {
        constexpr std::string_view chars = "0123456789ABCDEF";

        // Assure we always have an even number of chars since we decode each byte into a pair of chars.
        const std::size_t buffer_size = (width & 1) ? width + 1 : width;
        std::string result(buffer_size, '0');

        // For every byte starting at the LSB, decode into two chars and insert starting at the end of the string.
        // 'i' counts the number of hex digits, not the number of bytes.
        for (std::size_t i = 0; i < width; i += 2) {
            result[buffer_size - i - 1] = chars[(value >> (4 * i + 0)) & 0x0F];
            result[buffer_size - i - 2] = chars[(value >> (4 * i + 4)) & 0x0F];
        }

        // If for some reason width is an odd number, we have to erase one of the chars of the most significant byte.
        if (width & 1) [[unlikely]] result.erase(0, 1);
        return result;
    }


    /**
     * Converts the provided number to a string with the provided thousand-separator inserted.
     * @param value The number to convert to a string.
     * @param separator The thousand-separator to insert while converting the number.
     * @return The string representation of value with the provided thousand-separator grouping every set of 3 digits.
     */
    template <std::integral T> [[nodiscard]] inline std::string thousand_separate(T value, std::string_view separator) {
        std::string result = std::to_string(value);
        if (result.empty()) return result;

        std::size_t distance_to_sep = 0;
        for (std::size_t i = result.size() - 1; i > 0; --i) {
            if (std::isdigit(result[i - 1])) {
                if (++distance_to_sep == 3) {
                    result.insert(i, separator);
                    distance_to_sep = 0;
                }
            } else break;
        }

        return result;
    }


    /**
     * Finds the largest possible SI suffix for the given number and returns a pair [remainder, suffix].
     * E.g. converts 10'300 to [10.3 'K'], 1'000'000.5 to [1.0000005, 'M'], 1e9 to [1, 'G'] etc.
     * Numbers in the range (-1000, 1000) are returned unchanged with a space as their suffix.
     * @param value A number to convert
     * @return A pair [remainder, si_suffix] representing the given number.
     */
    template <numeric T> [[nodiscard]] constexpr inline std::pair<double, char> make_si_suffixed(T value) {
        constexpr std::string_view suffixes = " KMGTPEZYRQ";

        if constexpr (std::is_signed_v<T>) {
            if (value < 0) {
                auto [v, s] = make_si_suffixed((std::make_unsigned_t<T>) std::abs(value));
                return { -v, s };
            }
        }

        T previous;
        for (std::size_t suffix = 0; suffix < suffixes.length(); ++suffix) {
            if (value < 1e3) {
                return { (double) value, suffixes[suffix] };
            } else {
                previous = std::exchange(value, value / 1e3);
            }
        }

        return { previous, suffixes.back() };
    }


    /** Invokes @ref make_si_suffixed and constructs a string from the result by concatenating the digits and the suffix. */
    template <numeric T> [[nodiscard]] inline std::string make_si_suffixed_string(T value, std::size_t digits) {
        const auto [number, suffix] = make_si_suffixed(value);
        return std::format("{0:.{1}f}{2}", number, digits, suffix);
    }
}