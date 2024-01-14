#pragma once

#include <string>
#include <string_view>


// Range-v3
namespace ranges::views {}
namespace ranges::actions {}


namespace ve {
    namespace views   { using namespace ranges::views;   }
    namespace actions { using namespace ranges::actions; }

    using namespace std::string_literals;
    using namespace std::string_view_literals;
}