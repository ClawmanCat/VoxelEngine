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


// Glew must be included before gl.h, but it is included somewhere in the project. (Probably from SDL.)
#if VE_GRAPHICS_API == opengl
    #include <gl/glew.h>
#endif


namespace ve {
    namespace defs {
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        using namespace std::chrono_literals;

        namespace fs = std::filesystem;
    }

    using namespace defs;
}