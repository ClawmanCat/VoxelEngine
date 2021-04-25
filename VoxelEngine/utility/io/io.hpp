#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/color.hpp>
#include <VoxelEngine/utility/io/io_utils.hpp>
#include <VoxelEngine/utility/io/paths.hpp>

#include <lodepng.h>

#include <vector>
#include <fstream>


namespace ve::io {
    enum class write_mode { REPLACE = std::ios::trunc, APPEND = std::ios::app };
    
    using text_file = std::vector<std::string>;
    using data_file = std::vector<u8>;
    
    struct image {
        std::vector<color> pixels;
        vec2ui size;
    };
    
    
    
    // Filesystem interaction methods
    
    inline void mkdirs(const fs::path& path) {
        fs::create_directories(path.parent_path());
    }
    
    
    inline void create_required_paths(void) {
        for (const auto& path : paths::get_required_paths()) mkdirs(path);
    }
    
    
    // Unlike path.extension(), this method returns the full extensions for paths with multiple dot characters,
    // e.g. my_shader.vert.glsl becomes .vert.glsl and not .glsl.
    inline std::string full_extension(const fs::path& path) {
        std::string path_string = path.string();
        std::size_t last_dot = path_string.size();
        
        for (i64 i = path_string.size() - 1; i >= 0; --i) {
            if (path_string[i] == '.') last_dot = (u64) i;
            if (path_string[i] == '/' || path_string[(u64) i] == '\\') break;
        }
        
        return path_string.substr(last_dot, std::string::npos);
    }
    
    
    // File IO Methods
    
    inline expected<text_file> read_text(const fs::path& path) {
        std::ifstream stream(path);
        if (stream.fail()) return detail::get_unexpected("open stream", path);
        
        std::vector<std::string> result;
        std::string line;
        
        while (std::getline(stream, line)) result.push_back(std::move(line));
        
        return stream.eof() // Failbit will always be set since last call to getline will fail.
               ? make_expected(std::move(result))
               : detail::get_unexpected("read file", path);
    }
    
    
    inline expected<data_file> read_data(const fs::path& path) {
        std::ifstream stream(path, std::ios::binary);
        if (stream.fail()) return detail::get_unexpected("open stream", path);
        
        std::vector<u8> result;
        result.resize(detail::get_remaining_stream_size(stream));
        stream.read((char*) result.data(), result.size());
        
        return stream
               ? make_expected(std::move(result))
               : detail::get_unexpected("read file", path);
    }
    
    
    inline optional<exception_t> write_text(const fs::path& path, const text_file& text, write_mode mode = write_mode::REPLACE) {
        mkdirs(path);
        
        std::ofstream stream(path, (std::ios::openmode) mode);
        if (stream.fail()) return detail::get_unexpected("open stream", path).value();
        
        for (const auto& line : text) stream << line << '\n';
        
        return stream
            ? nullopt
            : ve::make_optional(detail::get_unexpected("write to file", path).value());
    }
    
    
    inline optional<exception_t> write_data(const fs::path& path, const data_file& data, write_mode mode = write_mode::REPLACE) {
        mkdirs(path);
        
        std::ofstream stream(path, std::ios::binary | (std::ios::openmode) mode);
        if (stream.fail()) return detail::get_unexpected("open stream", path).value();
        
        stream.write((const char*) data.data(), data.size());
        
        return stream
            ? nullopt
            : ve::make_optional(detail::get_unexpected("write to file", path).value());
    }
    
    
    inline expected<image> read_png(const fs::path& path) {
        // Assert no padding bytes exist that would cause errors when using memcpy to initialize the image.
        static_assert(sizeof(color) == 4 * sizeof(u8));
        
        // Read file and propagate any errors.
        auto bytes = read_data(path);
        if (!bytes.has_value()) return make_unexpected(bytes.error());
        
        // Decode PNG into std::vector<ImageChannel>.
        vec2ui size;
        std::vector<u8> channels;
        
        auto error_code = lodepng::decode(channels, size.x, size.y, *bytes);
        if (error_code) return detail::get_unexpected("decode PNG image", path, "LodePNG error code "s + std::to_string(error_code));
        
        // Merge channels into RGBA objects.
        std::vector<color> pixels;
        pixels.resize(size.x * size.y);
        memcpy(&pixels.front(), &channels.front(), sizeof(color) * size.x * size.y);
        
        return image { std::move(pixels), size };
    }
}