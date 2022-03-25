#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/string.hpp>
#include <VoxelEngine/utility/io/image.hpp>
#include <VoxelEngine/utility/assert.hpp>
#include <VoxelEngine/utility/algorithm.hpp>

#include <stb_image.h>
#include <stb_image_write.h>

#include <fstream>
#include <cerrno>


namespace ve::io {
    namespace detail {
        inline std::string get_error_string(std::string_view action, const fs::path& file, bool check_errno = true) {
            return check_errno
                ? cat("Failed to ", action, " for file ", file, ": ", strerror(errno))
                : cat("Failed to ", action, " for file ", file, ".");
        }


        inline u64 get_file_size(std::ifstream& stream) {
            auto current = stream.tellg();

            stream.seekg(0, std::ios::beg);
            auto start = stream.tellg();
            stream.seekg(0, std::ios::end);
            auto end = stream.tellg();

            stream.seekg(current, std::ios::beg);
            return (u64) (end - start);
        }
    }


    enum class write_mode { OVERWRITE = std::ios::trunc, APPEND = std::ios::app };

    using text_file = std::vector<std::string>;
    using data_file = std::vector<u8>;

    struct io_error : public std::runtime_error { using std::runtime_error::runtime_error; };


    inline text_file read_text(const fs::path& path) {
        std::ifstream stream { path };
        if (stream.fail()) throw io_error { detail::get_error_string("open file stream", path) };

        text_file result;
        std::string line;

        while (std::getline(stream, line)) result.push_back(std::move(line));

        if (!stream.eof()) throw io_error { detail::get_error_string("read file", path) };
        return result;
    }


    inline void write_text(const fs::path& path, const text_file& text, write_mode mode = write_mode::OVERWRITE) {
        fs::create_directories(path.parent_path());

        std::ofstream stream { path, (std::ios::openmode) mode };
        if (stream.fail()) throw io_error { detail::get_error_string("open file stream", path) };

        for (const auto& line : text) stream << line << '\n';

        if (!stream) throw io_error { detail::get_error_string("write file", path) };
    }


    inline data_file read_data(const fs::path& path) {
        std::ifstream stream { path, std::ios::binary };
        if (stream.fail()) throw io_error { detail::get_error_string("open file stream", path) };

        data_file result;
        result.resize(detail::get_file_size(stream));

        stream.read((char*) result.data(), (std::streamsize) result.size());

        if (!stream.eof()) throw io_error { detail::get_error_string("read file", path) };
        return result;
    }


    inline void write_data(const fs::path& path, const data_file& data, write_mode mode = write_mode::OVERWRITE) {
        fs::create_directories(path.parent_path());

        std::ofstream stream { path, std::ios::binary | (std::ios::openmode) mode };
        if (stream.fail()) throw io_error { detail::get_error_string("open file stream", path) };

        stream.write((const char*) data.data(), (std::streamsize) data.size());

        if (!stream) throw io_error { detail::get_error_string("write file", path) };
    }


    inline image_rgba8 load_image(const fs::path& path) {
        i32 w, h;

        auto ptr = stbi_load(path.string().c_str(), &w, &h, nullptr, 4);
        if (!ptr) throw io_error { detail::get_error_string("read image", path, false) };

        RGBA8* begin = reinterpret_cast<RGBA8*>(ptr);
        RGBA8* end   = begin + (w * h);

        image_rgba8 result {
            .data = std::vector<RGBA8> { begin, end },
            .size = vec2ui { (u32) w, (u32) h }
        };

        stbi_image_free(ptr);
        return result;
    }


    inline void write_image(const fs::path& path, const image_rgba8& image) {
        auto path_str = path.string();
        auto ext      = path.extension();

        if      (ext == ".png") stbi_write_png(path_str.c_str(), (i32) image.size.x, (i32) image.size.y, 4, image.data.data(), (i32) (sizeof(RGBA8) * image.size.x));
        else if (ext == ".bmp") stbi_write_bmp(path_str.c_str(), (i32) image.size.x, (i32) image.size.y, 4, image.data.data());
        else if (ext == ".jpg") stbi_write_jpg(path_str.c_str(), (i32) image.size.x, (i32) image.size.y, 4, image.data.data(), 100);
        else VE_ASSERT(false, "Unsupported texture format: ", ext.string().c_str());
    }


    // Unlike fs::path::extension, this method looks for the firstmost dot rather than the last one.
    // E.g. my_shader.vert.glsl will return .vert.glsl, rather than just .glsl as returned by the built-in method.
    inline std::string get_full_extension(const fs::path& path) {
        std::string path_string = fs::absolute(path).string();

        auto last_dot = path_string.end();
        for (auto it = path_string.rbegin(); it != path_string.rend(); ++it) {
            if (*it == '.') last_dot = it.base();
            if (one_of(*it, '\\', '/')) break;
        }

        return last_dot == path_string.end()
            ? ""
            : path_string.substr(std::distance(path_string.begin(), last_dot) - 1);
    }


    // Get the filename from a path, with the same considerations as the above method.
    inline std::string get_filename_from_multi_extension(const fs::path& path) {
        std::string ext  = get_full_extension(path);
        std::string name = path.filename().string();

        return name.substr(0, name.size() - ext.size());
    }
}