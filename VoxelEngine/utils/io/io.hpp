#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/logger.hpp>
#include <VoxelEngine/utils/io/paths.hpp>
#include <VoxelEngine/utils/color.hpp>

#include <lodepng.h>

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
        
        
        inline std::nullopt_t log_and_return(std::string_view action, fs::path file, std::optional<std::string_view> reason = std::nullopt) {
            if (is_in_directory(file, paths::ROOT_DIR)) {
                file = fs::relative(file, paths::ROOT_DIR);
            }
            
            if (!reason.has_value()) reason = strerror(errno);
            
            VE_LOG_ERROR("Failed to "s + action + " for " + file.string() + ": " + reason.value());
            return std::nullopt;
        }
    }
    
    
    using text_file = std::vector<std::string>;
    using data_file = std::vector<u8>;
    
    struct image {
        std::vector<color> pixels;
        vec2ui size;
    };
    
    
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
        
        return stream.eof() // Failbit will always be set since last call to getline will fail.
            ? std::optional { std::move(result) }
            : detail::log_and_return("read file", path);
    }
    
    
    inline std::optional<data_file> read_data(const fs::path& path) {
        std::ifstream stream(path, std::ios::binary);
        if (stream.fail()) return detail::log_and_return("open stream", path);
    
        std::vector<u8> result;
        result.resize(detail::get_remaining_stream_size(stream));
        stream.read((char*) result.data(), result.size());
        
        return stream
            ? std::optional { std::move(result) }
            : detail::log_and_return("read file", path);
    }
    
    
    // TODO: Expand for other formats & support non-RGBA PNGs.
    inline std::optional<image> read_png(const fs::path& path) {
        // Assert no padding bytes exist that would cause errors.
        static_assert(sizeof(color) == 4 * sizeof(u8));
        
        
        auto bytes = read_data(path);
        if (!bytes.has_value()) return std::nullopt;
        
        
        vec2ui size;
        std::vector<u8> channels;
        
        
        auto error_code = lodepng::decode(channels, size.x, size.y, *bytes);
        if (error_code) return detail::log_and_return("decode PNG image", path, "LodePNG error code "s + std::to_string(error_code));
        
        
        // Merge channels into RGBA objects.
        std::vector<color> pixels;
        pixels.resize(size.x * size.y);
        memcpy(&pixels.front(), &channels.front(), sizeof(color) * size.x * size.y);
        
        
        return image { std::move(pixels), size };
    }
    
    
    inline bool write_text(const fs::path& path, const text_file& text, write_mode mode = write_mode::REPLACE) {
        io::mkdirs(path);
        
        std::ofstream stream(path, (std::ios::openmode) mode);
        if (stream.fail()) return detail::log_and_return("open stream", path), false;
        
        for (const auto& line : text) stream << line << '\n';
        
        return (bool) stream ?: (detail::log_and_return("write to file", path), false);
    }
    
    
    inline bool write_data(const fs::path& path, const data_file& data, write_mode mode = write_mode::REPLACE) {
        io::mkdirs(path);
        
        std::ofstream stream(path, std::ios::binary | (std::ios::openmode) mode);
        if (stream.fail()) return detail::log_and_return("open stream", path), false;
        
        stream.write((const char*) data.data(), data.size());
        
        return (bool) stream ?: (detail::log_and_return("write to file", path), false);
    }
    
    
    inline void create_required_paths(void) {
        for (const auto& path : paths::get_required_paths()) io::mkdirs(path);
    }
    
    
    // Unlike path.extension(), this method returns the full extensions for paths with multiple dot characters,
    // e.g. my_shader.vert.glsl becomes .vert.glsl and not .glsl.
    inline std::string full_extension(const fs::path& path) {
        std::string path_string = path.string();
        std::size_t last_dot = path_string.size();
        
        for (std::size_t i = path_string.size() - 1; i >= 0; --i) {
            if (path_string[i] == '.') last_dot = i;
            if (path_string[i] == '/' || path_string[i] == '\\') break;
        }
        
        return path_string.substr(last_dot, std::string::npos);
    }
}