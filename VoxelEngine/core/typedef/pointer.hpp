#pragma once

#include <memory>


namespace ve {
    namespace defs {
        template <typename T, typename Deleter = std::default_delete<T>>
        using unique = std::unique_ptr<T, Deleter>;
        
        template <typename T> using shared = std::shared_ptr<T>;
        template <typename T> using weak   = std::weak_ptr<T>;
        
        
        using std::make_unique;
        using std::make_shared;


        struct weak_nullptr_t {
            template <typename T> constexpr operator weak<T>(void) const {
                return weak<T>{};
            }

            template <typename T> constexpr bool operator==(const weak<T>& other) const {
                return other.expired();
            }

            template <typename T> constexpr bool operator!=(const weak<T>& other) const {
                return !other.expired();
            }
        };

        constexpr weak_nullptr_t weak_nullptr;


        template <typename Class, typename Checked> requires (
            std::is_polymorphic_v<Checked> &&
            (std::is_base_of_v<Class, Checked> || std::is_base_of_v<Checked, Class>)
        ) constexpr bool instanceof(const Checked* ptr) {
            return dynamic_cast<const Class*>(ptr) != nullptr;
        }
    }
    
    using namespace defs;
}