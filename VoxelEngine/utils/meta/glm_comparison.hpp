#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/meta/traits/glm_traits.hpp>


namespace ve {
    template <typename VecA, typename VecB> requires meta::glm_traits<VecA>::is_vector && meta::glm_traits<VecB>::is_vector
    constexpr inline auto operator<(const VecA& a, const VecB& b) {
        return glm::lessThan(a, b);
    }
    
    
    template <typename VecA, typename VecB> requires meta::glm_traits<VecA>::is_vector && meta::glm_traits<VecB>::is_vector
    constexpr inline auto operator<=(const VecA& a, const VecB& b) {
        return glm::lessThanEqual(a, b);
    }
    
    
    template <typename VecA, typename VecB> requires meta::glm_traits<VecA>::is_vector && meta::glm_traits<VecB>::is_vector
    constexpr inline auto operator>(const VecA& a, const VecB& b) {
        return glm::greaterThan(a, b);
    }
    
    
    template <typename VecA, typename VecB> requires meta::glm_traits<VecA>::is_vector && meta::glm_traits<VecB>::is_vector
    constexpr inline auto operator>=(const VecA& a, const VecB& b) {
        return glm::greaterThanEqual(a, b);
    }
}