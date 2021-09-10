#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/vector.hpp>


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
}