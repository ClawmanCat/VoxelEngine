#pragma once

#include <VoxelEngine/core/core.hpp>

#include <compare>


#define ve_make_comparable(cls)                                                 \
[[nodiscard]] constexpr auto operator<=>(const cls&) const noexcept = default;  \
[[nodiscard]] constexpr bool operator== (const cls&) const noexcept = default;  \
[[nodiscard]] constexpr bool operator!= (const cls&) const noexcept = default;


#define ve_make_eq_comparable(cls)                                              \
[[nodiscard]] constexpr bool operator==(const cls&) const noexcept = default;   \
[[nodiscard]] constexpr bool operator!=(const cls&) const noexcept = default;