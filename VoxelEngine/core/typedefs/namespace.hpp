#pragma once

#include <range/v3/all.hpp>

#include <string>
#include <string_view>
#include <chrono>
#include <filesystem>


namespace ve {
    namespace namespaces {
        using namespace std::string_literals;
        using namespace std::string_view_literals;
        using namespace std::chrono_literals;
        using namespace std::chrono;
        
        namespace views   = ranges::views;
        namespace actions = ranges::actions;
        namespace fs      = std::filesystem;
    }
    
    using namespace namespaces;
}