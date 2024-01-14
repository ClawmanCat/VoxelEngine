#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/string.hpp>

#include <range/v3/range.hpp>
#include <gtest/gtest.h>

#include <type_traits>
#include <regex>


namespace ve::detail {
    inline bool regex_match(std::string_view text, std::string_view regex) {
        return std::regex_match(text.begin(), text.end(), std::regex { regex.begin(), regex.end() });
    }


    template <std::size_t N, typename... Args>
    constexpr inline auto get_nth(Args&&... args) {
        return std::get<N>(std::tuple { fwd(args)... });
    }


    template <std::size_t N, typename... Args>
    constexpr inline auto get_nth_typename(void) {
        return ve::typename_of<std::tuple_element_t<N, std::tuple<Args...>>>();
    }
}


#define VE_IMPL_TYPE_ASSERT(Assertion, ...)                                 \
Assertion((std::is_same_v<__VA_ARGS__>))                                    \
    << "\nT1: " << ve::detail::get_nth_typename<0, __VA_ARGS__>()           \
    << "\nT2: " << ve::detail::get_nth_typename<1, __VA_ARGS__>()


#define ASSERT_TYPE_EQ(...) VE_IMPL_TYPE_ASSERT(ASSERT_TRUE, __VA_ARGS__)
#define ASSERT_TYPE_NE(...) VE_IMPL_TYPE_ASSERT(ASSERT_FALSE, __VA_ARGS__)

#define EXPECT_TYPE_EQ(...) VE_IMPL_TYPE_ASSERT(EXPECT_TRUE, __VA_ARGS__)
#define EXPECT_TYPE_NE(...) VE_IMPL_TYPE_ASSERT(EXPECT_FALSE, __VA_ARGS__)


// While GTest has built-in regex support, it does not support a lot of regex features, while std::regex does.
#define ASSERT_MATCHES(...)                                                 \
ASSERT_TRUE(ve::detail::regex_match(__VA_ARGS__))                           \
    << "\nString: " << ve::detail::get_nth<0>(__VA_ARGS__)                  \
    << "\nDid not match regex: " << ve::detail::get_nth<1>(__VA_ARGS__)

#define EXPECT_MATCHES(...)                                                 \
EXPECT_TRUE(ve::detail::regex_match(__VA_ARGS__))                           \
    << "\nString: " << ve::detail::get_nth<0>(__VA_ARGS__)                  \
    << "\nDid not match regex: " << ve::detail::get_nth<1>(__VA_ARGS__)


// Asserts the equality of two ranges.
#define ASSERT_RANGE_EQ(...)                                                                    \
ASSERT_TRUE(ranges::equal(__VA_ARGS__))                                                         \
    << "\nRange 1: [" << ve::cat_range_with(", ", ve::detail::get_nth<0>(__VA_ARGS__)) << "]"   \
    << "\nRange 2: [" << ve::cat_range_with(", ", ve::detail::get_nth<1>(__VA_ARGS__)) << "]";

#define EXPECT_RANGE_EQ(...)                                                                    \
EXPECT_TRUE(ranges::equal(__VA_ARGS__))                                                         \
    << "\nRange 1: [" << ve::cat_range_with(", ", ve::detail::get_nth<0>(__VA_ARGS__)) << "]"   \
    << "\nRange 2: [" << ve::cat_range_with(", ", ve::detail::get_nth<1>(__VA_ARGS__)) << "]";

