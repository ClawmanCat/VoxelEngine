#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve::serialize {
    // Encodes length as a variable number of bytes, depending on its value and appends the result to dest.
    // The most significant bit in every byte is set to zero, except for the first one, which is set to one.
    // This is similar to how multi-byte encodings like UTF-8 work,
    // except in reverse since the decode method pops from the end of the array.
    inline void encode_variable_length(u64 length, std::vector<u8>& dest) {
        u64 remaining = length;

        while (true) {
            u8 byte = u8(remaining) & 0b0111'1111;
            byte |= u8(remaining == length) * 0b1000'0000;
            remaining >>= 7;

            dest.push_back(byte);
            if (remaining == 0) break;
        }
    }


    // Decodes a length encoded by encode_variable_length at the end of the source array,
    // and pops the read bytes from that array.
    inline u64 decode_variable_length(std::span<const u8>& source) {
        u64 result = 0;

        while (true) {
            u8 current = take_back(source);

            result <<= 7;
            result |= current & 0b0111'1111;

            if (current & 0b1000'0000) break;
        }

        return result;
    }


    // Used with boost asio to check if a message containing a variable length integer has been fully transferred.
    template <typename Ctr> struct transfer_variable_length_t {
        const Ctr* destination;

        std::size_t operator()(const auto& error, std::size_t transferred) const {
            return error || (!destination->empty() && destination->back() & 0b1000'0000) ? 0 : 1;
        }
    };

    template <typename Ctr> inline auto transfer_variable_length(const Ctr& destination) {
        return transfer_variable_length_t<Ctr> { &destination };
    }
}