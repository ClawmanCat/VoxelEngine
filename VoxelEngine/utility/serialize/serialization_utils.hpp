#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Simple serialization utility for memcpying a bunch of objects into a byte array.
    struct push_serializer {
        std::vector<u8> bytes;
        
        explicit push_serializer(std::size_t capacity = 0) {
            bytes.reserve(capacity);
        }
        
        template <typename T> requires std::is_trivial_v<T>
        void push(const T& value, std::size_t size = sizeof(T)) {
            bytes.resize(bytes.size() + size);
            memcpy(&bytes[bytes.size() - size], &value, size);
        }
        
        void push_bytes(std::span<u8> new_bytes) {
            bytes.insert(bytes.end(), new_bytes.begin(), new_bytes.end());
        }
    };
    
    
    // Simple serialization utility for memcpying a byte array into a bunch of objects.
    struct pop_deserializer {
        std::span<u8> bytes;
        
        explicit pop_deserializer(std::span<u8> bytes) : bytes(bytes) {}
        
        template <typename T>
        void pop_into(T& value, std::size_t size = sizeof(T)) {
            memcpy(&value, &bytes.front(), size);
            bytes = bytes.subspan(size);
        }
        
        template <typename T>
        T pop(void) {
            T result;
            pop_into(result);
            return result;
        }
        
        std::span<u8> pop_bytes(std::size_t count) {
            std::span<u8> result { bytes.begin(), bytes.begin() + count };
            bytes = bytes.subspan(count);
            return result;
        }
    };
}