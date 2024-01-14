#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/services/random.hpp>

#include <range/v3/all.hpp>

#include <vector>
#include <string>
#include <string_view>
#include <tuple>
#include <optional>


/** Value generators for operation_fuzzer */
namespace ve::testing::fuzz_arguments {
    template <std::integral T> class random_int {
    public:
        random_int(void) = default;
        random_int(T min, T max) : min_value(min), max_value(max) {}

        [[nodiscard]] T operator()(void) const {
            return generator.random_int<T>(min_value, max_value);
        }

    private:
        T min_value = ve::min_value<T>;
        T max_value = ve::max_value<T>;
        mutable random::fast_random_generator generator;
    };


    template <std::floating_point T> class random_float {
    public:
        random_float(void) = default;
        random_float(T min, T max) : min_value(min), max_value(max) {}

        [[nodiscard]] T operator()(void) const {
            return generator.random_real<T>(min_value, max_value);
        }

    private:
        T min_value = ve::min_value<T>;
        T max_value = ve::max_value<T>;
        mutable random::fast_random_generator generator;
    };


    template <typename T> class from_value_set {
    public:
        template <ranges::sized_range R> explicit from_value_set(const R& range) : values(range | ranges::to<std::vector>) {}
        explicit from_value_set(std::initializer_list<T> list) : values(list) {}

        [[nodiscard]] T operator()(void) const {
            return generator.random_choice(values);
        }

    private:
        std::vector<T> values;
        mutable random::fast_random_generator generator;
    };

    template <typename R> from_value_set(const R&) -> from_value_set<ranges::value_type_t<R>>;


    class random_string {
    public:
        constexpr static inline std::string_view charset_alpha_upper = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        constexpr static inline std::string_view charset_alpha_lower = "abcdefghijklmnopqrstuvwxyz";
        constexpr static inline std::string_view charset_numeric     = "0123456789";


        random_string(std::size_t min_length, std::size_t max_length, std::string charset) :
            min_length(min_length),
            max_length(max_length),
            charset(std::move(charset))
        {}


        [[nodiscard]] std::string operator()(void) const {
            std::size_t length = generator.random_int(min_length, max_length);

            std::string result;
            result.reserve(length);

            for (std::size_t i = 0; i < length; ++i) result += generator.random_choice(charset);

            return result;
        }

    private:
        std::size_t min_length, max_length;
        std::string charset;
        mutable random::fast_random_generator generator;
    };


    template <typename Generator, typename... Generators> requires (std::is_same_v<std::invoke_result_t<Generator>, std::invoke_result_t<Generators>> && ...)
    class multi_generator {
    public:
        using value_type = std::invoke_result_t<Generator>;


        explicit multi_generator(Generator g0, Generators... gs) : generators { std::move(g0), std::move(gs)... } {}


        [[nodiscard]] value_type operator()(void) const {
            std::size_t target = generator_picker.random_int(0ull, std::tuple_size_v<decltype(generators)> - 1);
            std::optional<value_type> result;

            tuple_foreach(generators, [&, i = 0ull] (const auto& gen) mutable {
                if (target == i++) result = gen();
            });

            return std::move(*result);
        }
    private:
        std::tuple<Generator, Generators...> generators;
        mutable random::fast_random_generator generator_picker;
    };
}