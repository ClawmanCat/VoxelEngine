#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>


namespace ve {
    struct bimap_kv_tag_t {};
    struct bimap_vk_tag_t {};


    template <
        typename K,
        typename V,
        template <typename...> typename Map = hash_map,
        typename... MapArgs
    > class bimap {
    public:
        using left_t  = Map<K, V, MapArgs...>;
        using right_t = Map<V, K, MapArgs...>;

        const left_t&  left(void)  const { return left_map; }
        const right_t& right(void) const { return right_map; }


        void clear(void) {
            left_map.clear();
            right_map.clear();
        }


        template <typename KT, typename VT> requires (
            std::is_same_v<K, std::remove_cvref_t<KT>> &&
            std::is_same_v<V, std::remove_cvref_t<VT>>
        ) void insert(KT&& k, VT&& v) {
            left_map.insert({ k, v });
            right_map.insert({ fwd(v), fwd(k) });
        }


        template <typename KT, typename VT> requires (
            std::is_same_v<K, std::remove_cvref_t<KT>> &&
            std::is_same_v<V, std::remove_cvref_t<VT>>
        ) void insert_or_assign(KT&& k, VT&& v) {
            left_map.insert_or_assign({ k, v });
            right_map.insert_or_assign({ fwd(v), fwd(k) });
        }


        void erase_key(const K& key) {
            auto it = left_map.find(key);

            right_map.erase(it->second);
            left_map.erase(it);
        }


        void erase_key(typename left_t::const_iterator it) {
            right_map.erase(it->second);
            left_map.erase(it);
        }


        void erase_val(const V& val) {
            auto it = right_map.find(val);

            left_map.erase(it->second);
            right_map.erase(it);
        }


        void erase_val(typename right_t::const_iterator it) {
            left_map.erase(it->second);
            right_map.erase(it);
        }


        typename left_t::iterator find_key(const K& key) { return left_map.find(key); }
        typename left_t::const_iterator find_key(const K& key) const { return left_map.find(key); }

        typename right_t::iterator find_val(const V& val) { return right_map.find(val); }
        typename right_t::const_iterator find_val(const V& val) const { return right_map.find(val); }


        bool contains_key(const K& key) const {
            return left_map.contains(key);
        }

        bool contains_val(const V& val) const {
            return right_map.contains(val);
        }

    private:
        left_t left_map;
        right_t right_map;
    };
}