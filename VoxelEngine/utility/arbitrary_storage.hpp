#pragma once

#include <VoxelEngine/core/core.hpp>

#include <any>


namespace ve {
    // Note: hash function is implemented as the hash_combine of all keys and values.
    // Objects that are not hashable will hash as zero and thus can cause hashes to compare equal.
    class arbitrary_storage {
    public:
        template <typename T>
        T& store_object(std::string name, T value) {
            auto hash = [&] {
                if constexpr (requires (T t) { std::hash<T>{}(t); }) {
                    static_assert(!std::is_same_v<T, fs::path>, "A");
                    return hash_of(value);
                }
                else return std::size_t { 0 };
            }();

            auto [it, success] = storage.emplace(std::move(name), value_t { std::move(value), hash });
            return std::any_cast<T&>(it->second.value);
        }


        template <typename T>
        T& store_or_replace_object(std::string name, T value) {
            auto hash = [&] {
                if constexpr (requires (T t) { std::hash<T>{}(t); }) {
                    static_assert(!std::is_same_v<T, fs::path>, "A");
                    return hash_of(value);
                }
                else return std::size_t { 0 };
            }();

            auto [it, was_newly_inserted] = storage.insert_or_assign(std::move(name), value_t { std::move(value), hash });
            return std::any_cast<T&>(it->second.value);
        }


        void remove_object(std::string_view name) {
            storage.erase(name);
        }


        template <typename T>
        T take_object(std::string_view name) {
            T object = std::move(std::any_cast<T&>(storage.extract(name).mapped().value));
            return object;
        }


        template <typename T>
        T& get_object(std::string_view name) {
            return std::any_cast<T&>(storage.at(name).value);
        }


        template <typename T>
        const T& get_object(std::string_view name) const {
            return std::any_cast<const T&>(storage.at(name).value);
        }


        template <typename T>
        T* try_get_object(std::string_view name) {
            return has_object(name) ? &get_object<T>(name) : nullptr;
        }


        template <typename T>
        const T* try_get_object(std::string_view name) const {
            return has_object(name) ? &get_object<T>(name) : nullptr;
        }


        // Note: unlike get_object, this method copies the value out of the storage if it exists!
        template <typename T>
        T object_or(std::string_view name, T&& default_value) const {
            return has_object(name) ? get_object<T>(name) : fwd(default_value);
        }


        template <typename T>
        T& get_or_create_object(std::string_view name, auto&&... construction_args) {
            if (auto it = storage.find(name); it != storage.end()) {
                return std::any_cast<T&>(it->second.value);
            }

            return store_object(std::string { name }, T(fwd(construction_args)...));
        }


        bool has_object(std::string_view name) const {
            return storage.contains(name);
        }


        std::size_t hash(void) const {
            std::size_t result = 0;

            for (const auto& [k, v] : storage) {
                hash_combine(result, k);
                hash_combine(result, v.hash);
            }

            return result;
        }
    private:
        struct value_t { std::any value; std::size_t hash; };
        stable_hash_map<std::string, value_t> storage;
    };

}