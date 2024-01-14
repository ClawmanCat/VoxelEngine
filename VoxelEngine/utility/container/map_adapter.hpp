#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/container/insert_iterator_adapter.hpp>

#include <range/v3/algorithm.hpp>

#include <vector>


namespace ve {
    /**
     * Wrapper around a sequence container to give it a map-like interface without changing the way the underlying data is stored.
     * @warning This container does not automatically validate the uniqueness of its keys if methods from the base class are used to insert elements!
     *
     * @tparam K The type of keys stored in this container.
     * @tparam V The type of value stored in this container.
     * @tparam Container The underlying sequence container.
     * @tparam Inserter An insert iterator type to add items to the container with.
     *  - The inserter must be constructible from a pointer to the container.
     *  - Insertion happens by dereferencing and assigning to the iterator, i.e. (*it) = elem should insert into the container.
     *  - The inserter must also provide a method where() to retrieve an iterator to the most recently inserted element.
     * @tparam Eq Comparator for keys.
     */
    template <
        typename K,
        typename V,
        template <typename...> typename Container = std::vector,
        template <typename...> typename Inserter  = retrievable_back_insert_iterator,
        typename Eq = default_eq<K>
    > class map_adapter : public Container<std::pair<K, V>> {
    public:
        using base_type   = Container<std::pair<K, V>>;
        using inserter    = Inserter<base_type>;
        using key_type    = K;
        using mapped_type = V;
        using key_equal   = Eq;

        VE_WRAP_TYPEDEFS(
            base_type,
            value_type, size_type, difference_type,
            reference, const_reference, pointer, const_pointer,
            iterator, const_iterator, reverse_iterator, const_reverse_iterator
        );


        [[nodiscard]] bool contains(const K& key) const {
            return find(key) != base_type::end();
        }


        [[nodiscard]] V& at(const K& key) {
            if (auto it = find(key); it != base_type::end()) [[likely]] return it->second;
            throw std::out_of_range { "No such key." };
        }

        [[nodiscard]] const V& at(const K& key) const {
            if (auto it = find(key); it != base_type::end()) [[likely]] return it->second;
            throw std::out_of_range { "No such key." };
        }


        [[nodiscard]] V& operator[](const K& key) {
            if (auto it = find(key); it != base_type::end()) [[likely]] return it->second;
            insert(value_type { key, V{} });
        }

        [[nodiscard]] const V& operator[](const K& key) const {
            if (auto it = find(key); it != base_type::end()) [[likely]] return it->second;
            insert(value_type { key, V{} });
        }


        [[nodiscard]] iterator find(const K& key) {
            return ranges::find(*this, key, &value_type::first);
        }

        [[nodiscard]] const_iterator find(const K& key) const {
            return ranges::find(*this, key, &value_type::first);
        }


        std::pair<iterator, bool> insert(value_type value) {
            if (auto it = find(value.first); it != base::end()) {
                return { it, false };
            } else {
                inserter i { *this };

                i = std::move(value);
                return { i.where(), true };
            }
        }


        bool erase(const K& key) {
            return (bool) std::erase_if(*this, [&] (const auto& kv) { return kv.first == key; });
        }
    };
}