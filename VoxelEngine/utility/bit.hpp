#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


namespace ve {
    namespace detail {
        template <auto Value> struct can_contain {
            template <typename T> struct type {
                constexpr static inline bool value = (Value >= min_value<T> && Value <= max_value<T>);
            };
        };
    }


    /**
     * Returns the smallest integral type that can contain the given value.
     * If the value is less than 0 a signed type is returned, otherwise an unsigned type is returned.
     * @tparam Value The value that should be containable within the returned integer type.
     */
    template <auto Value> using smallest_integral_type_for = decltype([] {
        using unsigned_ts = meta::pack<u8, u16, u32, u64>;
        using signed_ts   = meta::pack<i8, i16, i32, i64>;

        if constexpr (Value >= 0) {
            return typename unsigned_ts
                ::template filter_trait<detail::can_contain<Value>::template type>
                ::head {};
        } else {
            return typename signed_ts
                ::template filter_trait<detail::can_contain<Value>::template type>
                ::head {};
        }
    } ());


    /**
     * Returns an instance of integer type T with 'count' bits set to 1, starting at 'offset' bits from the LSB.
     * @param count The number of bits to set.
     * @param offset The offset at which to start setting bits, from the LSB.
     * @return An integer of type T with 'count' bits set to 1 at the provided offset.
     */
    template <std::unsigned_integral T> [[nodiscard]] constexpr inline T set_n_bits(u8 count, u8 offset = 0) {
        T value { 0 };

        for (u8 bit = offset; bit < offset + count; ++bit) {
            value |= (1ull << bit);
        }

        return value;
    }
}