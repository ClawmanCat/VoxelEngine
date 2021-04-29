#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/math.hpp>
#include <VoxelEngine/utility/traits/any_of.hpp>

#include <random>
#include <array>
#include <bit>


namespace ve::random {
    // Uses the hardware random device to generate a sequence of random numbers of size N,
    // to be used as the seed for a pseudo-random number generator.
    template <std::size_t N> inline std::array<u8, N> generate_seed(void) {
        std::array<u8, N> result;
        std::random_device source;
        
        std::generate(result.begin(), result.end(), std::ref(source));
        
        return result;
    }
    
    
    template <typename Gen> constexpr inline std::size_t generator_state_size =
        Gen::state_size * sizeof(typename Gen::result_type);
    
    
    template <
        typename Gen,
        std::size_t State = generator_state_size<Gen>
    > inline auto create_seeded_generator(const std::array<u8, State>& seed) {
        std::seed_seq seed_seq(seed.begin(), seed.end());
        return Gen { seed_seq };
    }
    
    
    namespace cheap {
        // Very cheap random number generator.
        class xorshift {
        public:
            using result_type = u64;
            
            
            explicit xorshift(u64 seed = 0ull) : state(seed) {}
            explicit xorshift(const std::array<u8, sizeof(u64)>& seed) : state(std::bit_cast<u64>(seed)) {}
            
            
            u64 operator()(void) {
                state ^= state << 13;
                state ^= state >> 7;
                state ^= state << 17;
                
                return state;
            }
            
            
            static u64 min(void) { return min_value<u64>; }
            static u64 max(void) { return max_value<u64>; }
        private:
            u64 state;
        };
        
        
        inline thread_local xorshift global_generator { generate_seed<sizeof(u64)>() };
    
    
        // Produces a random floating point number with uniform distribution between min and max.
        template <typename T = float> requires std::is_floating_point_v<T>
        inline T random_real(T min = T(0.0), T max = T(1.0)) {
            std::uniform_real_distribution<T> dis { min, max };
            return dis(global_generator);
        }
    
        // Produces a random integer number with uniform distribution between min and max (inclusive).
        template <typename T = u32> requires std::is_integral_v<T>
        inline T random_int(T min = min_value<T>, T max = max_value<T>) {
            std::uniform_int_distribution<T> dis { min, max };
            return dis(global_generator);
        }
    
        // Produces a random floating point number with normal distribution the given mean and standard deviation.
        template <typename T = float> requires std::is_floating_point_v<T>
        inline T random_normal(T mean = T(0), T stddev = T(1.0)) {
            std::normal_distribution<T> dis { mean, stddev };
            return dis(global_generator);
        }
    }
    
    
    namespace good {
        inline thread_local std::mt19937 global_generator = create_seeded_generator<std::mt19937>(
            generate_seed<generator_state_size<std::mt19937>>()
        );
    
    
        // Produces a random floating point number with uniform distribution between min and max.
        template <typename T = float> requires std::is_floating_point_v<T>
        inline T random_real(T min = T(0.0), T max = T(1.0)) {
            std::uniform_real_distribution<T> dis { min, max };
            return dis(global_generator);
        }
    
        // Produces a random integer number with uniform distribution between min and max (inclusive).
        template <typename T = u32> requires std::is_integral_v<T>
        inline T random_int(T min = min_value<T>, T max = max_value<T>) {
            std::uniform_int_distribution<T> dis { min, max };
            return dis(global_generator);
        }
    
        // Produces a random floating point number with normal distribution the given mean and standard deviation.
        template <typename T = float> requires std::is_floating_point_v<T>
        inline T random_normal(T mean = T(0), T stddev = T(1.0)) {
            std::normal_distribution<T> dis { mean, stddev };
            return dis(global_generator);
        }
    }
}


namespace ve {
    namespace cheaprand = random::cheap;
    namespace goodrand  = random::good;
}