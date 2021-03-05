#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/io/paths.hpp>

#include <filesystem>
#include <string_view>
#include <cerrno>
#include <fstream>


namespace ve::io::detail {
    // Checks if the given path is within the given directory, either directly or indirectly.
    inline bool is_in_directory(fs::path p, const fs::path& root_dir) {
        while (!fs::equivalent(p, p.parent_path())) {
            if (fs::equivalent(p, root_dir)) return true;
            p = p.parent_path();
        }
        
        return false;
    }
    
    
    // Construct an expected object containing an error message for the given file and action.
    inline unexpected get_unexpected(std::string_view action, fs::path file, optional<std::string_view> reason = nullopt) {
        if (is_in_directory(file, paths::ROOT_DIR)) {
            file = fs::relative(file, paths::ROOT_DIR);
        }
        
        return make_unexpected(
            "Failed to "s + action +
            " for " + file.string() +
            ": " + reason.value_or(strerror(errno))
        );
    }
    
    
    // Gets the distance between the current read position and the end of the stream.
    inline u64 get_remaining_stream_size(std::ifstream& stream) {
        u64 current = stream.tellg();
        stream.seekg(0, std::ios::end);
        u64 end = stream.tellg();
        stream.seekg(current, std::ios::beg);
    
        return end - current;
    }
}