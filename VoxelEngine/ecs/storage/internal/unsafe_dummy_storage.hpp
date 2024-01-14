#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/ecs/component/component_traits.hpp>


namespace ve::ecs::detail {
    /**
     * Drop-in replacement for @ref unsafe_paged_storage for objects which can be storage-eluded.
     * These are objects that are empty (no data members) and whose destruction is unobservable (trivial).
     */
    template <storage_eludable_component_type T> class unsafe_dummy_storage {
    public:
        constexpr static inline bool trivially_destructible = true;
        constexpr static inline bool is_copyable            = true;
        constexpr static inline bool is_movable             = true;


        template <typename... Args> T& emplace(std::size_t index, Args&&... args) {
            // Since T is trivially destructible it is not UB to overwrite it here, even if its lifetime has already started.
            new (value.buffer) T { fwd(args)... };
            return value.get();
        }


        void erase(std::size_t index) {}
        void clear(void) {}


        T&       operator[](std::size_t index)       { return value.get(); }
        const T& operator[](std::size_t index) const { return value.get(); }


        void shrink_to_fit(void) {}
    private:
        struct dummy_object {
            alignas(T) u8 buffer[sizeof(T)];
            T& get(void) const { return *((T*) buffer); }
        };

        static inline dummy_object value;
    };
}