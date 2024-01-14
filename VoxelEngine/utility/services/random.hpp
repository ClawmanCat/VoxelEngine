#pragma once

#include <VoxelEngine/core/core.hpp>

#include <range/v3/all.hpp>

#include <random>


namespace ve::random {
    /**
     * Very fast random number generator good enough for most non-secure purposes.
     * This class meets the "RandomNumberEngine" named requirement, with the exception of not having streaming operators.
     * Based on: G. Marsaglia, “Xorshift RNGs”, J. Stat. Soft., vol. 8, no. 14, pp. 1–6, Jul. 2003.
     */
    class xorshift_generator {
    public:
        using result_type = u64;


        xorshift_generator(void) { seed(); }
        explicit xorshift_generator(result_type state) { seed(state); }
        explicit xorshift_generator(auto& seq) { seed(seq); }


        [[nodiscard]] constexpr bool operator==(const xorshift_generator& o) const = default;


        [[nodiscard]] constexpr static result_type min(void) { return min_value<result_type>; }
        [[nodiscard]] constexpr static result_type max(void) { return max_value<result_type>; }


        [[nodiscard]] constexpr result_type operator()(void) {
            u64 prev_state = state;

            state ^= prev_state << 13;
            state ^= prev_state >> 7;
            state ^= prev_state << 17;

            return state * 0x2545F4914F6CDD1DULL;
        }


        void seed(result_type seed = 0) {
            state = seed - result_type(seed == 0); // Prevent seed from being 0.
        }


        template <typename T> requires requires (T seq) { seq.generate(); }
        void seed(T& seq) {
            seed(seq.generate());
        }


        void discard(result_type count) {
            for (std::size_t i = 0; i < count; ++i) std::ignore = (*this)();
        }
    private:
        result_type state;
    };


    /**
     * Wrapper around a RandomNumberEngine to provide various utility functions and preserve engine state.
     * @tparam Engine The underlying random number engine.
     */
    template <typename Engine> class random_number_generator {
    public:
        explicit random_number_generator(Engine engine = Engine { }) : engine(std::move(engine)) { reseed(); }


        /** Reseeds the RNG with the provided seed. */
        void reseed(auto value) { engine.seed(value); }
        /** Reseeds the RNG with a random (hardware-generated) seed. */
        void reseed(void) { reseed(std::random_device{}()); }


        /**
         * Generates a random integer of the provided type between 'min' and 'max'.
         * @param min Inclusive lower bound for the generated random number. Defaults to min_value<T>.
         * @param max Inclusive upper bound for the generated random number. Defaults to max_value<T>.
         * @return A random integer chosen uniformly from the interval [min, max].
         */
        template <std::integral T = i64> [[nodiscard]] inline T random_int(T min = min_value<T>, T max = max_value<T>) {
            std::uniform_int_distribution<T> dist { min, max };
            return dist(engine);
        }


        /**
         * Generates a random floating point number of the provided type between 'min' and 'max'.
         * @param min Inclusive lower bound for the generated random number. Defaults to 0.
         * @param max Inclusive upper bound for the generated random number. Defaults to 1.
         * @return A random floating point number chosen uniformly from the interval [min, max].
         */
        template <std::floating_point T = f64> [[nodiscard]] inline T random_real(T min = T(0.0), T max = T(1.0)) {
            std::uniform_real_distribution<T> dist { min, max };
            return dist(engine);
        }


        /**
         * Generates a random floating point number of the provided type based on the normal distribution centered on 'mean' with standard deviation 'stddev'.
         * @param mean The mean of the normal distribution, i.e. the number on which the distribution is centered. Defaults to 0.
         * @param stddev The standard deviation of the normal distribution, i.e. the square root of the average distance squared distance to the mean. Defaults to 1.
         * @return A random floating point number chosen uniformly from the interval [min, max].
         */
        template <std::floating_point T = f64> [[nodiscard]] inline T random_normal(T mean = T(0.0), T stddev = T(1.0)) {
            std::normal_distribution<T> dist { mean, stddev };
            return dist(engine);
        }


        /**
         * Selects a random element from the provided range. The provided range must be a sized range.
         * @param range A sized range from which to select a random element.
         * @return A randomly chosen element from the provided range.
         */
        template <ranges::sized_range Range> [[nodiscard]] inline decltype(auto) random_choice(const Range& range) {
            auto index = random_int<std::size_t>(0, ranges::size(range) - 1);
            return ranges::at(range, index);
        }


        /** @copydoc random_choice */
        template <typename T> [[nodiscard]] inline auto& random_choice(std::initializer_list<T> list) {
            return random_choice(ranges::subrange(list.begin(), list.end()));
        }


        /** Randomly returns either true or false. */
        [[nodiscard]] inline bool random_bool(void) {
            return random_choice({ true, false });
        }


        VE_GET_MREFS(engine);
    private:
        Engine engine;
    };


    /** RNG Service using a fast generator at the cost of lower quality random numbers. */
    struct fast_random_generator : random_number_generator<xorshift_generator> {
        VE_SERVICE(fast_random_generator);
    };

    /** RNG Service using a better generator than @ref fast_random_generator, at the cost of being much slower. */
    struct good_random_generator : random_number_generator<std::mt19937_64> {
        VE_SERVICE(good_random_generator);
    };
}