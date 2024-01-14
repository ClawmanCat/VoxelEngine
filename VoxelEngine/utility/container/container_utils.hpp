#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/container_traits.hpp>
#include <VoxelEngine/utility/meta/forward_as.hpp>
#include <VoxelEngine/utility/meta/sequence.hpp>
#include <VoxelEngine/utility/meta/value.hpp>
#include <VoxelEngine/utility/meta/stop_token.hpp>

#include <range/v3/all.hpp>

#include <array>
#include <type_traits>
#include <tuple>


namespace ve {
    /**
     * Utility concept to allow iterators of the form It<Parent> where Parent is const for const iterators to be convertible
     * from iterator to const iterator.
     * @tparam Other The parent of the iterator type to construct this iterator from.
     * @tparam Self The parent of the current iterator type.
     */
    template <typename Other, typename Self> concept iterator_compatible_parent =
        std::is_same_v<std::remove_cvref_t<Other>, std::remove_cvref_t<Self>> &&
        (std::is_const_v<Self> || !std::is_const_v<Other>);


    /**
     * Removes and returns the last element of the provided container. Container must support the pop_back() operation.
     * @param container A container for which container.pop_back() is valid.
     * @return The now-removed last element of the container.
     */
    template <typename C> requires requires (C container) { container.pop_back(); }
    inline typename meta::container_value_type<C> take_back(C& container) {
        auto value = std::move(*std::prev(std::end(container)));
        container.pop_back();
        return value;
    }


    /**
     * Removes and returns the first element of the provided container. Container must support the pop_front() operation.
     * @param container A container for which container.pop_front() is valid.
     * @return The now-removed first element of the container.
     */
    template <typename C> requires requires (C container) { container.pop_front(); }
    inline typename meta::container_value_type<C> take_front(C& container) {
        auto value = std::move(*std::begin(container));
        container.pop_front();
        return value;
    }


    /**
     * Removes and returns the element with the given index/key from the provided container. Container must support the operations container[key] and container.erase(key).
     * @param container A container from which to remove the given element.
     * @param key The key to the element to remove.
     * @return The now-removed element of the container that had the given key.
     */
    template <typename C, typename K> requires requires (C container, K key) { container[key], container.erase(key); }
    inline auto take_at(C& container, const K& key) {
        auto value = std::move(container[key]);
        container.erase(key);
        return value;
    }


    /**
     * Invokes 'function' on each element of the provided tuple-like object 'tpl'. @ref stop_token can be used with this method.
     * @param tpl An object for which std::get<N>(tpl) and std::tuple_size<decltype(tpl)> are valid.
     * @param function A function to invoke for each element of the tuple. Elements are passed to this function with a reference-type equal to that of 'tpl'.
     * @return The return type of this function depends on that of fn:
     *  - If fn returns meta::stop_token for any value, the return type of this function is std::variant<R..., meta::null_type>,
     *    where R... are all possible stop token value types returned by this function.
     *    The value of this variant is the payload of the first stop token for which stop = true, or null_type if no such token existed.
     *  - In all other cases, the return type of this function is void.
     */
    template <typename Tpl, typename Fn> constexpr inline auto tuple_foreach(Tpl&& tpl, Fn&& function) {
        using tuple_type = std::remove_reference_t<Tpl>;

        return meta::invoke_stop_token_fn<meta::pack, 0, std::tuple_size_v<tuple_type>>([&] <std::size_t I> (meta::value<I>) {
            return std::invoke(function, std::get<I>(tpl));
        });
    }


    /**
     * Returns a new tuple containing the elements of the given tuple with indices in the range [From, From + Count].
     * @tparam From The index of the first element of the tuple to include in the result tuple.
     * @tparam Count The number of elements from the source tuple to include in the result tuple.
     * @param tpl A tuple
     * @return A tuple of the elements from 'tpl' with indices in the range [From, From + Count].
     */
    template <std::size_t From, std::size_t Count, typename Tpl> requires (From + Count <= std::tuple_size_v<std::remove_cvref_t<Tpl>>)
    constexpr inline auto tuple_slice(Tpl&& tpl) {
        return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
            return std::tuple<std::tuple_element_t<Is, Tpl>...> {
                std::get<Is>(fwd(tpl))...
            };
        } (meta::make_integer_sequence<std::size_t, From, From + Count, 1>());
    }


    /**
     * Creates a std::array of N elements filled with copies of 'value'.
     * @tparam N The size of the array to generate.
     * @param value The value to fill the array with.
     * @return An array of size N filled with N copies of 'value'.
     */
    template <std::size_t N, typename T> constexpr inline std::array<T, N> create_filled_array(const T& value) {
        auto get_value = [&] (std::size_t) -> const auto& { return value; };

        return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
            return std::array<T, N> { get_value(Is)... };
        } (std::make_index_sequence<N>());
    }


    /**
     * Creates a std::array of N elements filled by invoking generator(meta::value<I>) for each element index I.
     * @tparam N The size of the array to generate.
     * @param generator The generator to generate values to fill the array with.
     * @return An array of size N filled with the result of invoking generator for each index.
     */
    template <std::size_t N, typename F, typename T = std::invoke_result_t<F, meta::value<0ull>>>
    constexpr inline std::array<T, N> generate_filled_array(F&& generator) {
        auto get_value = [&] <std::size_t I> (meta::value<I> index) -> T { return std::invoke(generator, index); };

        return [&] <std::size_t... Is> (std::index_sequence<Is...>) {
            return std::array { get_value(meta::value<Is>{})... };
        } (std::make_index_sequence<N>());
    }


    /** Equivalent to ranges::contains, but also works for std::initializer_list, allowing code like is_one_of({ X, Y, Z }, value). */
    template <typename Rng> constexpr inline bool is_one_of(const Rng& range, const auto& value) {
        return ranges::contains(range, value);
    }

    /** @copydoc is_one_of */
    template <typename T> constexpr inline bool is_one_of(const std::initializer_list<T>& list, const T& value) {
        return is_one_of(ranges::subrange(list.begin(), list.end()), value);
    }


    /**
     * Constructs a std::array of reference wrappers to the provided elements.
     * Useful for creating a container on-the-fly to iterate over several distinct variables.
     * @param args A set of objects of a common type to create an array of references to.
     * @return A std::array of std::reference_wrappers to each element of args.
     */
    template <typename... Args> constexpr inline auto refs(Args&&... args) {
        return std::array<
            std::reference_wrapper<std::remove_reference_t<std::common_type_t<decltype(args)...>>>,
            sizeof...(Args)
        > { std::ref(args)... };
    }
}