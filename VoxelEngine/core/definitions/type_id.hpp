#pragma once

#include <VoxelEngine/core/platform.hpp>

#include <crc_cpp.h>

#include <cstddef>
#include <compare>
#include <string_view>


namespace ve {
    namespace detail {
        /**
         * Constexpr string hashing function. Uses CRC64 as the hash of the provided value.
         * (Yes this is slow but we're only calculating it at compile time anyway.)
         */
        consteval inline u64 constexpr_string_hash(std::string_view sv) {
            crc_cpp::crc64_ecma crc;
            for (const char ch : sv) crc.update(static_cast<u8>(ch));
            return crc.final();
        }


        consteval inline std::string_view extract_typename(std::string_view source, std::size_t prefix, std::size_t suffix) {
            source.remove_prefix(prefix);
            source.remove_suffix(suffix);
            return source;
        }


        template <typename T> consteval inline std::string_view typename_of(void) {
            using namespace std::string_view_literals;
            std::size_t prefix = 0, suffix = 0;
            std::string_view source;

            // Cannot use std::source_location: MSVC does not provide the template parameter in the function name.
            #if defined(VE_COMPILER_MSVC)
                prefix = "ve::detail::typename_of<"sv.size();
                suffix = ">(void)"sv.size();
                source = __FUNCSIG__;
            #elif defined(VE_COMPILER_CLANG)
                prefix = "std::string_view ve::detail::typename_of() [T = "sv.size();
                suffix = "]"sv.size();
                source = __PRETTY_FUNCTION__;
            #elif defined(VE_COMPILER_GCC)
                prefix = "consteval std::string_view ve::detail::typename_of() [with T = "sv.size();
                suffix = "; std::string_view = std::basic_string_view<char>]"sv.size();
                source = __PRETTY_FUNCTION__;
            #endif

            return extract_typename(source, prefix, suffix);
        }
    }


    class type_id_t {
    public:
        consteval type_id_t(std::string_view type_name) :
            type_name(type_name),
            hash_value(detail::constexpr_string_hash(type_name))
        {}

        [[nodiscard]] constexpr auto operator<=>(const type_id_t&) const = default;
        [[nodiscard]] constexpr bool operator== (const type_id_t&) const = default;

        constexpr std::size_t hash(void) const { return hash_value; }
        constexpr std::string_view name(void) const { return type_name; }
    private:
        std::string_view type_name;
        std::size_t hash_value;
    };


    class type_index_t {
    public:
        consteval type_index_t(const type_id_t& id) : hash_value(id.hash()) {}

        [[nodiscard]] constexpr auto operator<=>(const type_index_t&) const = default;
        [[nodiscard]] constexpr bool operator== (const type_index_t&) const = default;

        constexpr std::size_t hash(void) const { return hash_value; }
    private:
        std::size_t hash_value;
    };


    /** Returns a type_id object uniquely representing the given type. Constexpr-equivalent of std::type_info. */
    template <typename T> consteval inline type_id_t        type_id    (void) { return type_id_t { detail::typename_of<T>() }; }
    /** Returns a type_index object representing an unique ID for the given type. Constexpr-equivalent of std::type_index. */
    template <typename T> consteval inline type_index_t     type_index (void) { return type_index_t { type_id<T>() }; }
    /** Returns an implementation-defined string representing the name of the provided type. */
    template <typename T> consteval inline std::string_view typename_of(void) { return type_id<T>().name(); }
    /** Returns an implementation-defined hash of the provided type. */
    template <typename T> consteval inline std::size_t      typehash_of(void) { return type_id<T>().hash(); }
}