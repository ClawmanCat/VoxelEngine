#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utils/priority.hpp>
#include <VoxelEngine/utils/container_utils.hpp>
#include <VoxelEngine/utils/iteratable.hpp>

#include <magic_enum.hpp>

#include <vector>
#include <array>
#include <cstddef>
#include <algorithm>
#include <optional>
#include <type_traits>
#include <iterator>


namespace ve::events {
    // Vector which preserves the ordering of elements with different priorities.
    // Ordering between elements with the same priority is not preserved.
    // Insert and erase have time complexities linear to the number of priorities.
    template <typename T, typename Priority = priority>
    requires (
        std::is_enum_v<Priority> &&
        !std::is_same_v<magic_enum::underlying_type_t<Priority>, u64> &&       // Enum values must be storable as signed value.
        !magic_enum::enum_contains<Priority>(-1)                               // Reserved value.
    )
    class priority_storage {
    public:
        constexpr static std::size_t num_priorities = magic_enum::enum_count<Priority>();
        constexpr static i64 no_values_for_priority = -1;
        
        
        priority_storage(void) { indices.fill(no_values_for_priority); }
        
        
        // Since the ordering between elements of the same priority is irrelevant,
        // we can just repeatedly swap elements with the first element of the next priority,
        // and then insert the final swapped element at the end.
        void insert(T&& value, Priority p) {
            std::size_t pid = (std::size_t) p;
            
            std::optional<std::size_t> insert_pos = std::nullopt;
            for (std::size_t next = pid + 1; next < indices.size(); ++next) {
                if (indices[next] == no_values_for_priority) continue;
                
                if (!insert_pos.has_value()) insert_pos = indices[next];
                std::swap(value, storage[indices[next]]);
                ++indices[next];
            }
    
            storage.push_back(std::move(value));
            if (indices[pid] == no_values_for_priority) indices[pid] = insert_pos.value_or(storage.size() - 1);
        }
        
        
        void insert(const T& value, Priority p) {
            insert(T { value }, p);
        }
        
        
        // Finds and erases the first element from the container for which pred holds true.
        // Returns whether or not the container was modified.
        template <typename Pred> bool erase_if(Pred&& pred) {
            auto it = std::find_if(storage.begin(), storage.end(), pred);
            if (it == storage.end()) return false;
            
            i64 erase_index = std::distance(storage.begin(), it);
            auto& swap_element = *it;
            
            
            // If this is not the last element with this priority, swap it so that it is.
            std::size_t next = std::distance(indices.begin(), std::upper_bound(indices.begin(), indices.end(), erase_index)) - 1;
            if (indices[next] - 1 > erase_index) std::swap(swap_element, storage[indices[next]]);
            
            
            // Swap the last element of the priority with the first element of the next priority,
            // until we reach the end of the container.
            for (; next < indices.size(); ++next) {
                if (indices[next] == no_values_for_priority) continue;
                
                std::swap(swap_element, storage[indices[next]]);
                --indices[next];
            }
            
            
            // The element that needs to be removed is now at the end of the container.
            storage.pop_back();
            return true;
        }
        
        
        void clear(void) {
            storage.clear();
            indices.fill(no_values_for_priority);
        }
        
    private:
        std::vector<T> storage;
        std::array<i64, num_priorities> indices;
        
    public:
        VE_MAKE_ITERATABLE(storage);
    };
}