#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/traits/glm_traits.hpp>

#include <glm/gtx/norm.hpp>


namespace ve::distance_functions {
    template <
        typename Derived,
        typename Vec,
        typename Distance = typename Vec::value_type,
        typename SquareDistance = typename Vec::value_type
    > struct distance_function {
        [[nodiscard]] constexpr Distance distance(const Vec& a, const Vec& b) const {
            VE_CRTP_CHECK(Derived, distance);
            return static_cast<const Derived*>(this)->distance(a, b);
        }
        
        
        [[nodiscard]] constexpr SquareDistance distance_squared(const Vec& a, const Vec& b) const {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, distance_squared)) {
                return static_cast<const Derived*>(this)->distance_squared(a, b);
            } else {
                float dist = distance(a, b);
                return dist * dist;
            }
        }
    
    
        [[nodiscard]] constexpr bool within(const Vec& a, const Vec& b, typename Vec::value_type distance) const {
            if constexpr (VE_CRTP_IS_IMPLEMENTED(Derived, within)) {
                return static_cast<const Derived*>(this)->within(a, b, distance);
            } else {
                return distance_squared(a, b) < square(distance);
            }
        }
    };
    
    
    template <typename Vec> struct euclidean : public distance_function<euclidean<Vec>, Vec, double> {
        [[nodiscard]] constexpr double distance(const Vec& a, const Vec& b) const {
            return constexpr_sqrt<double>(distance_squared(a, b));
        }
        
        
        [[nodiscard]] constexpr auto distance_squared(const Vec& a, const Vec& b) const {
            typename Vec::value_type result = 0;
            
            foreach_dimension<Vec>([&](std::size_t i){
                result += square(b[i] - a[i]);
            });
            
            return result;
        }
    };
    
    
    template <typename Vec> struct L1 : public distance_function<L1<Vec>, Vec> {
        [[nodiscard]] constexpr auto distance(const Vec& a, const Vec& b) const {
            typename Vec::value_type result = 0;
    
            foreach_dimension<Vec>([&](std::size_t i){
                result += std::abs(b[i] - a[i]);
            });
    
            return result;
        }
    };
}