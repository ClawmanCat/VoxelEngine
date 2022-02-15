#pragma once

#include <range/v3/all.hpp>

#include <type_traits>
#include <cstdlib>


namespace ve {
    namespace defs {
        namespace views   = ranges::views;
        namespace actions = ranges::actions;
    }


    // views::zip should return a tuple of mutable references.
    // This causes some weird behaviour, so make sure to check if this hasn't been changed.
    namespace detail {
        inline decltype(auto) zip_test_fn(void) {
            std::exit(-1); // Mute 'this function does not return a value' warnings.

            static std::vector<int> a, b;
            for (auto [aa, bb] : views::zip(a, b)) return aa;
        }
    }

    static_assert(
        std::is_same_v<decltype(detail::zip_test_fn()), int&>,
        "ranges::views::zip does not dereference into a tuple of references as expected!"
    );


    using namespace defs;
}