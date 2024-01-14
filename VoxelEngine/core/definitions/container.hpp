#pragma once

#include <VoxelEngine/core/definitions/hash.hpp>

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/container/node_hash_set.h>
#include <absl/container/btree_map.h>
#include <absl/container/btree_set.h>
#include <boost/container/small_vector.hpp>

#include <functional>


namespace ve {
    /** Default hasher used for engine hash containers. */
    template <typename K> using default_hash = ve::hasher<K>;
    /** Default equality operator used for engine containers. */
    template <typename K> using default_eq   = std::equal_to<K>;
    /** Default comparator used for engine containers. */
    template <typename K> using default_cmp  = std::less<K>;


    /** Tree-based map container, equivalent to std::map. */
    template <typename K, typename V, typename Cmp = default_cmp<K>>
    using tree_map = absl::btree_map<K, V, Cmp>;

    /** Tree-based set container, equivalent to std::set. */
    template <typename K, typename Cmp = default_cmp<K>>
    using tree_set = absl::btree_set<K, Cmp>;

    /** Tree-based multimap container, equivalent to std::multimap. */
    template <typename K, typename V, typename Cmp = default_cmp<K>>
    using tree_multimap = absl::btree_multimap<K, V, Cmp>;

    /** Tree-based multiset container, equivalent to std::multiset. */
    template <typename K, typename Cmp = default_cmp<K>>
    using tree_multiset = absl::btree_multiset<K, Cmp>;


    /** Hash-based map container, equivalent to std::unordered_map, but modifying the container always invalidates iterators. */
    template <typename K, typename V, typename Hash = default_hash<K>, typename Eq = default_eq<K>>
    using hash_map = absl::flat_hash_map<K, V, Hash, Eq>;

    /** Hash-based set container, equivalent to std::unordered_set, but modifying the container always invalidates iterators. */
    template <typename K, typename Hash = default_hash<K>, typename Eq = default_eq<K>>
    using hash_set = absl::flat_hash_set<K, Hash, Eq>;

    /** Hash-based map container, equivalent to std::unordered_map, but iterators are never invalidated. */
    template <typename K, typename V, typename Hash = default_hash<K>, typename Eq = default_eq<K>>
    using stable_hash_map = absl::node_hash_map<K, V, Hash, Eq>;

    /** Hash-based set container, equivalent to std::unordered_set, but iterators are never invalidated. */
    template <typename K, typename Hash = default_hash<K>, typename Eq = default_eq<K>>
    using stable_hash_set = absl::node_hash_set<K, Hash, Eq>;


    /** Equivalent to std::vector, but with a built-in buffer for the first N elements, to prevent unnecessary allocations. */
    template <typename T, std::size_t N> using small_vector = boost::container::small_vector<T, N>;

    /** Equivalent to std::vector, but with a constant-size buffer of size N, beyond which the size of the vector cannot grow. */
    template <typename T, std::size_t N> using static_vector = boost::container::static_vector<T, N>;
}