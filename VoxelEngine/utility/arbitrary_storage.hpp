#pragma once

#include <VoxelEngine/core/core.hpp>

#include <any>


namespace ve {
    class arbitrary_storage {
    public:
        template <typename T>
        T& store_object(std::string name, T&& value) {
            auto [it, success] = storage.emplace(std::move(name), std::move(value));
            return std::any_cast<T&>(it->second);
        }
    
    
        void remove_object(std::string_view name) {
            storage.erase(name);
        }
    
    
        template <typename T>
        T take_object(std::string_view name) {
            T object = std::move(std::any_cast<T&>(storage.extract(name).mapped()));
            return object;
        }
    
    
        template <typename T>
        T& get_object(std::string_view name) {
            return std::any_cast<T&>(storage.at(name));
        }
    
    
        template <typename T>
        const T& get_object(std::string_view name) const {
            return std::any_cast<const T&>(storage.at(name));
        }


        template <typename T>
        T& get_or_create_object(std::string_view name, auto&&... construction_args) {
            if (auto it = storage.find(name); it != storage.end()) {
                return std::any_cast<T&>(it->second);
            }

            return store_object(std::string { name }, T(fwd(construction_args)...));
        }

    
        bool has_object(std::string_view name) const {
            return storage.contains(name);
        }
    private:
        stable_hash_map<std::string, std::any> storage;
    };
}