#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/algorithm.hpp>


namespace ve::serialize {
    struct push_serializer {
        std::vector<u8>& bytes;


        template <typename T> requires std::is_trivial_v<T>
        void push(const T& object) {
            std::size_t old_size = bytes.size();
            bytes.resize(bytes.size() + sizeof(T));

            memcpy(&bytes[old_size], &object, sizeof(T));
        }


        template <typename Ctr>
        void push_bytes(const Ctr& ctr) {
            std::size_t old_size = bytes.size();
            bytes.resize(bytes.size() + ctr.size());

            std::copy(ctr.begin(), ctr.end(), bytes.begin() + old_size);
        }


        std::size_t size(void) const {
            return bytes.size();
        }
    };


    struct pop_deserializer {
        std::span<const u8> bytes;


        template <typename Ctr> explicit pop_deserializer(const Ctr& ctr)
            : bytes(ctr.begin(), ctr.end()) {}


        template <typename T> requires std::is_trivial_v<T>
        void pop_into(T& result) {
            memcpy(&result, bytes.last(sizeof(T)).data(), sizeof(T));
            take_back_n(bytes, sizeof(T));
        }


        template <typename T> requires std::is_trivial_v<T>
        T pop(void) {
            T result;
            pop_into(result);
            return result;
        }


        std::span<const u8> pop_bytes(std::size_t count) {
            return take_back_n(bytes, count);
        }


        void pop_bytes_into(std::vector<u8>& vec, std::size_t count) {
            std::size_t old_size = vec.size();
            vec.resize(vec.size() + count);

            memcpy(&vec[old_size], bytes.last(count).data(), count);
            take_back_n(bytes, count);
        }


        std::size_t size(void) const {
            return bytes.size();
        }
    };
}