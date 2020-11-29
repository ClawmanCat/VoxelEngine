#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/logger.hpp>
#include <VoxelEngine/utils/io/paths.hpp>

#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>
#include <cerrno>
#include <cstdio>


namespace ve::io {
    namespace detail {
        inline bool is_in_directory(fs::path p, const fs::path& root_dir) {
            while (!fs::equivalent(p, p.parent_path())) {
                if (fs::equivalent(p, root_dir)) return true;
                p = p.parent_path();
            }
            
            return false;
        }
        
        
        inline u64 get_remaining_stream_size(std::ifstream& stream) {
            u64 current = stream.tellg();
            stream.seekg(0, std::ios::end);
            u64 end = stream.tellg();
            stream.seekg(current, std::ios::beg);
            
            return end - current;
        }
        
        
        inline std::nullopt_t log_and_return(std::string_view action, fs::path file) {
            if (is_in_directory(file, paths::ROOT_DIR)) {
                file = fs::relative(file, paths::ROOT_DIR);
            }
            
            VE_LOG_ERROR("Failed to "s + action + " for " + file.string() + ": " + strerror(errno));
            return std::nullopt;
        }
    }
    
    
    using text_file = std::vector<std::string>;
    using data_file = std::vector<u8>;
    
    enum class write_mode { REPLACE = std::ios::trunc, APPEND = std::ios::app };
    
    
    inline void mkdirs(const fs::path& path) {
        fs::create_directories(path.parent_path());
    }
    
    
    inline std::optional<text_file> read_text(const fs::path& path) {
        std::ifstream stream(path);
        if (stream.fail()) return detail::log_and_return("open stream", path);
        
        std::vector<std::string> result;
        std::string line;
        
        while (std::getline(stream, line)) result.push_back(std::move(line));
        
        return stream.eof()
            ? std::optional { std::move(result) }
            : detail::log_and_return("read file", path);
    }
    
    
    inline std::optional<data_file> read_data(const fs::path& path) {
        std::ifstream stream(path, std::ios::binary);
        if (stream.fail()) return detail::log_and_return("open stream", path);
    
        std::vector<u8> result;
        result.resize(detail::get_remaining_stream_size(stream));
        stream.read((char*) result.data(), result.size());
        
        return stream.eof()
            ? std::optional { std::move(result) }
            : detail::log_and_return("read file", path);
    }
    
    
    inline bool write_text(const fs::path& path, const text_file& text, write_mode mode = write_mode::REPLACE) {
        io::mkdirs(path);
        
        std::ofstream stream(path, (std::ios::openmode) mode);
        if (stream.fail()) return detail::log_and_return("open stream", path), false;
        
        for (const auto& line : text) stream << line << '\n';
        
        return !stream.fail() ?: (detail::log_and_return("write to file", path), false);
    }
    
    
    inline bool write_data(const fs::path& path, const data_file& data, write_mode mode = write_mode::REPLACE) {
        io::mkdirs(path);
        
        std::ofstream stream(path, std::ios::binary | (std::ios::openmode) mode);
        if (stream.fail()) return detail::log_and_return("open stream", path), false;
        
        stream.write((const char*) data.data(), data.size());
        
        return !stream.fail() ?: (detail::log_and_return("write to file", path), false);
    }
    
    
    inline void create_required_paths(void) {
        for (const auto& path : paths::get_required_paths()) io::mkdirs(path);
    }
}