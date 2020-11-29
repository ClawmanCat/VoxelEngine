#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/traits.hpp>
#include <VoxelEngine/utils/io/io.hpp>

#include <fstream>
#include <filesystem>


namespace ve::io {
    // Creates an ofstream and all directories required for the ofstream to write to the given path.
    template <meta::is_loosely_any_of_c<const char*, const std::string&, const fs::path&, const fs::path::value_type*> Pathlike>
    inline std::ofstream mkdir_ofstream(Pathlike path, std::ios_base::openmode mode = std::ios::out) {
        mkdirs(fs::path(path));
        return std::ofstream(path, mode);
    }
}