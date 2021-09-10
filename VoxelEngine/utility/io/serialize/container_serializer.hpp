#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/always_false.hpp>
#include <VoxelEngine/utility/traits/pick.hpp>
#include <VoxelEngine/utility/traits/evaluate_if_valid.hpp>
#include <VoxelEngine/utility/traits/is_std_array.hpp>
#include <VoxelEngine/utility/io/serialize/variable_length_encoder.hpp>


namespace ve::serialize {
    template <typename T> void to_bytes(const T&, std::vector<u8>&);
    template <typename T> T from_bytes(std::span<const u8>&);


    // If a container cannot be inserted into with any of the default methods,
    // this struct can be specialized and provide the following members:
    //
    // static void insert(Container&, Element&&)
    // constexpr static bool insert_reversed
    //
    // If insert_reversed is true, insert is called in the reverse order elements were present in the original container.
    // This can be used for containers that only allow inserting at the front.
    template <typename T> struct container_inserter {
        using non_overloaded_tag = void;
    };


    namespace detail {
        template <typename T> using container_element_t  = std::remove_cvref_t<decltype(*std::cbegin(std::declval<T>()))>;
        template <typename T> using container_iterator_t = decltype(std::cbegin(std::declval<T>()));


        // Trigger a static assert if there is no way to insert into the given container.
        template <typename T> constexpr inline void trigger_assert_fail(void) {
            static_assert(
                meta::always_false_v<T>,
                "No known method to insert into container. "
                "Overload ve::serialize::container_inserter for this type to fix this error. "
                "(See " __FILE__ " for more information.)"
            );
        }


        // Returns whether or not elements should be inserted into container T in reverse order they were present in the original container.
        // This can be used for containers that only allow inserting at the back.
        template <typename T, typename E = container_element_t<T>> constexpr inline bool insert_reversed(void) {
            if constexpr (!requires { typename container_inserter<T>::non_overloaded_tag; }) {
                return container_inserter<T>::insert_reversed;
            }

            else if constexpr (requires (T t, E e) { t.push_front(std::move(e)); }) return false;
            else if constexpr (requires (T t, E e) { t.insert(std::move(e));     }) return false;
            else if constexpr (requires (T t, E e) { t.push_back(std::move(e));  }) return true;
            else if constexpr (requires (T t, E e) { t.push(std::move(e));       }) return true;

            else trigger_assert_fail<T>();
        }


        // Inserts the given element into the container.
        template <typename T, typename E = container_element_t<T>> constexpr inline void insert_element(T& ctr, E&& elem) {
            if constexpr (!requires { typename container_inserter<T>::non_overloaded_tag; }) {
                return container_inserter<T>::insert(ctr, fwd(elem));
            }

            else if constexpr (requires (T t, E e) { t.push_front(std::move(e)); }) ctr.push_front(fwd(elem));
            else if constexpr (requires (T t, E e) { t.insert(std::move(e));     }) ctr.insert(fwd(elem));
            else if constexpr (requires (T t, E e) { t.push_back(std::move(e));  }) ctr.push_back(fwd(elem));
            else if constexpr (requires (T t, E e) { t.push(std::move(e));       }) ctr.push(fwd(elem));

            else trigger_assert_fail<T>();
        }


        // If T has contiguous storage and its elements are trivial, just copy the entire storage
        // instead of serializing element by element.
        template <typename T> constexpr inline bool supports_trivial_container_serialization_v =
            requires (T t) { t.resize(); } &&
            requires (T t) { std::begin(t), std::end(t); } &&
            ve_eval_if_valid(std::contiguous_iterator<container_iterator_t<T>>) &&
            ve_eval_if_valid(std::is_trivial_v<container_element_t<T>>);


        // Can T be serialized using the container serializer?
        // Note: we cannot simply use requires { insert_element(t, e); } here,
        // since the static assert will escape the requires clause.
        // Note: std::array need not be handled here since it supports std::get and can be handled as a tuple like object.
        template <typename T> constexpr inline bool supports_container_serialization_v =
            supports_trivial_container_serialization_v<T> || (
                requires (T t) { std::cbegin(t), std::cend(t); } &&
                std::is_default_constructible_v<T> &&
                ve_eval_if_valid(
                    !requires { typename container_inserter<T>::non_overloaded_tag; } ||
                    requires (T t, container_element_t<T> e) { t.push_front(std::move(e)); } ||
                    requires (T t, container_element_t<T> e) { t.insert(std::move(e));     } ||
                    requires (T t, container_element_t<T> e) { t.push_back(std::move(e));  } ||
                    requires (T t, container_element_t<T> e) { t.push(std::move(e));       }
                )
            );
    }


    template <typename T> requires detail::supports_trivial_container_serialization_v<T>
    inline void trivial_container_to_bytes(const T& value, std::vector<u8>& dest) {
        std::span s { std::cbegin(value), std::cend(value) };

        std::size_t old_size = dest.size();
        dest.resize(dest.size() + s.size());

        memcpy(&dest[old_size], s.data(), s.size());
        encode_variable_length(dest.size() - old_size, dest);
    }


    template <typename T> requires detail::supports_trivial_container_serialization_v<T>
    inline T trivial_container_from_bytes(std::span<const u8>& src) {
        std::size_t count = decode_variable_length(src);

        T result { };
        result.resize(count);

        std::span s { std::begin(result), std::end(result) };
        memcpy(s.data(), take_back_n(src, count).data(), count);

        return result;
    }


    template <typename T> requires detail::supports_container_serialization_v<T>
    inline void container_to_bytes(const T& value, std::vector<u8>& dest) {
        // Faster alternative for trivially copyable data.
        if constexpr (detail::supports_trivial_container_serialization_v<T>) {
            trivial_container_to_bytes(value, dest);
            return;
        }


        auto view = meta::pick<detail::insert_reversed<T>()>(views::reverse, views::all);

        std::size_t count = 0;
        for (const auto& elem : value | view) {
            to_bytes<detail::container_element_t<T>>(elem, dest);
            ++count;
        }

        encode_variable_length(count, dest);
    }


    template <typename T> requires detail::supports_container_serialization_v<T>
    inline T container_from_bytes(std::span<const u8>& src) {
        // Faster alternative for trivially copyable data.
        if constexpr (detail::supports_trivial_container_serialization_v<T>) {
            return trivial_container_from_bytes<T>(src);
        }


        std::size_t count = decode_variable_length(src);

        T result { };
        for (std::size_t i = 0; i < count; ++i) {
            detail::insert_element(result, from_bytes<detail::container_element_t<T>>(src));
        }

        return result;
    }
}