#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <type_traits>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <optional>
#include <filesystem>
#include <functional>

#include <boost/uuid/uuid_hash.hpp>
#include <boost/uuid/uuid_io.hpp>


namespace ve {
    namespace defs {
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        using namespace std::chrono_literals;

        namespace fs = std::filesystem;
    }

    using namespace defs;
}