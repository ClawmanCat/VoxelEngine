#pragma once

#include <VoxelEngine/core/legacy.hpp>
#include <VoxelEngine/core/platform.hpp>

#include <boost/container/flat_map.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/small_vector.hpp>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/container/node_hash_set.h>
#include <absl/hash/hash.h>

// Abseil's btree performs a comparison with mismatched signs.
// There is nothing we can do about this, so just mute the warning.
#ifdef VE_COMPILER_CLANG
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wsign-compare"
    
    #include <absl/container/btree_map.h>
    #include <absl/container/btree_set.h>
    
    #pragma clang diagnostic pop
#endif


namespace ve {
    namespace defs {
        template <typename T, std::size_t N>
        using array_reference = T(&)[N];
        
        
        template <typename K, typename V, typename Cmp = std::less<K>>
        using tree_map = absl::btree_map<K, V, Cmp>;
    
        template <typename K, typename Cmp = std::less<K>>
        using tree_set = absl::btree_set<K, Cmp>;

        template <typename K, typename V, typename Cmp = std::less<K>>
        using tree_multimap = absl::btree_multimap<K, V, Cmp>;

        template <typename K, typename Cmp = std::less<K>>
        using tree_multiset = absl::btree_multiset<K, Cmp>;
    
    
        template <typename K> using default_hash = typename absl::flat_hash_set<K>::hasher;
        template <typename K> using default_eq   = typename absl::flat_hash_set<K>::key_equal;
    
        // Faster than std::unordered_[map|set] at the cost of pointer stability.
        template <typename K, typename V, typename Hash = default_hash<K>, typename Eq = default_eq<K>>
        using hash_map = absl::flat_hash_map<K, V, Hash, Eq>;
    
        template <typename K, typename Hash = default_hash<K>, typename Eq = default_eq<K>>
        using hash_set = absl::flat_hash_set<K, Hash, Eq>;
    
        // Alternative for flat_hash_[map|set] with pointer stability for both keys and values.
        template <typename K, typename V, typename Hash = default_hash<K>, typename Eq = default_eq<K>>
        using stable_hash_map = absl::node_hash_map<K, V, Hash, Eq>;
    
        template <typename K, typename Hash = default_hash<K>, typename Eq = default_eq<K>>
        using stable_hash_set = absl::node_hash_set<K, Hash, Eq>;

    
        // map / set interface on top of std::vector. Faster for very small containers.
        template <typename K, typename V, typename Cmp = std::less<K>>
        using vec_map = boost::container::flat_map<K, V, Cmp>;
    
        template <typename K, typename Cmp = std::less<K>>
        using vec_set = boost::container::flat_set<K, Cmp>;
    
        template <typename K, typename V, std::size_t PreAlloc = 8, typename Cmp = std::less<K>>
        using small_vec_map = boost::container::small_flat_map<K, V, PreAlloc, Cmp>;
    
        template <typename K, std::size_t PreAlloc = 8, typename Cmp = std::less<K>>
        using small_vec_set = boost::container::small_flat_set<K, PreAlloc, Cmp>;
    
    
        template <typename K, typename V, typename Cmp = std::less<K>>
        using vec_multimap = boost::container::flat_multimap<K, V, Cmp>;
    
        template <typename K, typename Cmp = std::less<K>>
        using vec_multiset = boost::container::flat_multiset<K, Cmp>;
    
        template <typename K, typename V, std::size_t PreAlloc = 8, typename Cmp = std::less<K>>
        using small_vec_multimap = boost::container::small_flat_multimap<K, V, PreAlloc, Cmp>;
    
        template <typename K, std::size_t PreAlloc = 8, typename Cmp = std::less<K>>
        using small_vec_multiset = boost::container::small_flat_multiset<K, PreAlloc, Cmp>;
    
    
        template <typename T, std::size_t PreAlloc = 8>
        using small_vector = boost::container::small_vector<T, PreAlloc>;

        template <typename T, std::size_t N>
        using static_vector = boost::container::static_vector<T, N>;

        template <typename T>
        using stable_vector = boost::container::stable_vector<T>;
    }
    
    using namespace defs;
}