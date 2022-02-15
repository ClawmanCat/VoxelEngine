#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/raii.hpp>
#include <VoxelEngine/utility/functional.hpp>
#include <VoxelEngine/utility/algorithm.hpp>
#include <VoxelEngine/utility/unit.hpp>

#define ZLIB_CONST
#include <zlib.h>


namespace ve {
    namespace detail {
        inline std::string stream_error_message(z_stream& stream, int error_code) {
            return cat(
                "ZLib internal error: ",
                stream.msg ? stream.msg : "no message",
                " (Error ", error_code, ")"
            );
        }
    }


    enum class compression_mode : int {
        DEFAULT          = Z_DEFAULT_COMPRESSION,
        NO_COMPRESSION   = Z_NO_COMPRESSION,
        BEST_PERFORMANCE = Z_BEST_SPEED,
        BEST_COMPRESSION = Z_BEST_COMPRESSION
    };


    inline std::vector<u8> compress(std::span<const u8> src, compression_mode mode = compression_mode::DEFAULT, u32 block_size = 64_kib) {
        if (src.empty()) [[unlikely]] return {};


        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree  = Z_NULL;
        stream.opaque = Z_NULL;

        stream.avail_in = src.size();
        stream.next_in  = (const Bytef*) src.data();

        // For very small inputs, its wasteful to allocate the full 64kib, so just allocate slightly more than the input size.
        // (For small inputs the output size could be greater than the input, since compression will be difficult and a header is added.)
        u32 initial_alloc = std::min((u32) src.size() + 128, block_size);
        std::vector<u8> dest(initial_alloc, 0x00);

        stream.avail_out = initial_alloc;
        stream.next_out  = dest.data();


        if (auto status = deflateInit(&stream, (int) mode); status != Z_OK) [[unlikely]] {
            throw std::runtime_error { detail::stream_error_message(stream, status) };
        }

        auto cleanup_on_exit = raii_function { no_op, [&] { deflateEnd(&stream); } };


        // Note: avail_in being zero does not mean we're done!
        // There may be some data left in the internal ZLib buffer that we need to flush afterwards.
        while (stream.avail_in > 0) {
            auto status = deflate(&stream, Z_NO_FLUSH);
            if (status != Z_OK) [[unlikely]] throw std::runtime_error { detail::stream_error_message(stream, status) };

            if (stream.avail_out == 0) {
                dest.resize(dest.size() + block_size, 0x00);
                stream.avail_out += block_size;

                // Resizing the vector may change the underlying data location.
                stream.next_out = dest.data() + stream.total_out;
            }
        }

        // Flush any remaining data.
        // Note: Z_DATA_ERROR is produced if the output buffer is not big enough to perform a complete flush.
        for (auto status = Z_OK; status != Z_STREAM_END; status = deflate(&stream, Z_FINISH)) {
            if (one_of(status, Z_OK, Z_BUF_ERROR)) {
                dest.resize(dest.size() + block_size, 0x00);
                stream.avail_out += block_size;

                // Resizing the vector may change the underlying data location.
                stream.next_out = dest.data() + stream.total_out;
            }

            if (!one_of(status, Z_OK, Z_BUF_ERROR, Z_STREAM_END)) [[unlikely]] {
                throw std::runtime_error { detail::stream_error_message(stream, status) };
            }
        }


        dest.resize(stream.total_out);
        return dest;
    }


    inline std::vector<u8> decompress(std::span<const u8> src, u32 block_size = 64_kib) {
        if (src.empty()) [[unlikely]] return {};


        z_stream stream;
        stream.zalloc = Z_NULL;
        stream.zfree  = Z_NULL;
        stream.opaque = Z_NULL;

        stream.avail_in = src.size();
        stream.next_in  = (const Bytef*) src.data();

        // For very small inputs, its wasteful to allocate the full 64kib, so just allocate a few times the input size.
        u32 initial_alloc = std::min((u32) src.size() * 4, block_size);
        std::vector<u8> dest(initial_alloc, 0x00);

        stream.avail_out = initial_alloc;
        stream.next_out  = dest.data();


        if (auto status = inflateInit(&stream); status != Z_OK) [[unlikely]] {
            throw std::runtime_error { detail::stream_error_message(stream, status) };
        }

        auto cleanup_on_exit = raii_function { no_op, [&] { inflateEnd(&stream); } };


        // Note: unlike deflate, inflate will return Z_STREAM_END even if we're not using Z_FINISH when everything is done.
        // However, we have no guarantee it will ever finish unless we use Z_FINISH to flush the data once we're done reading.
        bool finish = false;
        for (auto status = Z_OK; status != Z_STREAM_END; /* No Action */) {
            if (stream.avail_in == 0) finish = true;

            status = inflate(&stream, finish ? Z_FINISH : Z_NO_FLUSH);
            if (!one_of(status, Z_OK, Z_STREAM_END, Z_BUF_ERROR)) [[unlikely]] throw std::runtime_error { detail::stream_error_message(stream, status) };

            if (stream.avail_out == 0 || status == Z_BUF_ERROR) {
                dest.resize(dest.size() + block_size, 0x00);
                stream.avail_out += block_size;

                // Resizing the vector may change the underlying data location.
                stream.next_out = dest.data() + stream.total_out;
            }
        }


        dest.resize(stream.total_out);
        return dest;
    }
}