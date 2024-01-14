#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/meta/value.hpp>


namespace ve {
    template <typename GetRegistry, typename T> struct autoregister_type_t {
        const static inline meta::null_type on_init = [] {
            GetRegistry{}().template register_type<T>();
            return meta::null_type {};
        };
    };

    /** Automatically registers the type T in the type registry returned by GetRegistry{}() on program startup. */
    template <typename GetRegistry, typename T> inline auto autoregister_type = autoregister_type_t<GetRegistry, T>{};


    /**
     * Stores an object of type Data for every type T registered within this registry,
     * by invoking the provided generator with an argument of meta::type<T>{}.
     *
     * @tparam Data The type of data stored within this registry.
     * @tparam DataGenerator A type invocable as generator(meta::type<T>{}) returning an object of type Data.
     */
    template <typename Data, typename DataGenerator = typename Data::generator> class type_registry {
    public:
        type_registry(void) = default;
        explicit type_registry(DataGenerator generator) : generator(std::move(generator)) {}


        template <typename T> void register_type(void) {
            per_type_data.emplace(
                type_index<T>(),
                std::invoke(generator, meta::type<T>{})
            );
        }


        template <typename T> [[nodiscard]] bool contains(void) const {
            return per_type_data.contains(type_index<T>());
        }

        [[nodiscard]] bool contains(const type_index_t& key) const {
            return per_type_data.contains(key);
        }


        template <typename T> [[nodiscard]] Data& get(void) {
            return per_type_data.at(type_index<T>());
        }

        template <typename T> [[nodiscard]] const Data& get(void) const {
            return per_type_data.at(type_index<T>());
        }

        [[nodiscard]] Data& get(const type_index_t& key) {
            return per_type_data.at(key);
        }

        [[nodiscard]] const Data& get(const type_index_t& key) const {
            return per_type_data.at(key);
        }
    private:
        hash_map<type_index_t, Data> per_type_data;
        VE_NO_UNIQUE_ADDRESS DataGenerator generator;
    };
}