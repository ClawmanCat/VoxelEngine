#pragma once

#include <VoxelEngine/core/core.hpp>


namespace ve {
    // Equivalent to std::erase_if, but accepts a predicate that takes an iterator, rather than a value.
    template <typename Container, typename Pred>
    void erase_if_it(Container& ctr, Pred pred) {
        auto first = ctr.begin();
        auto last  = ctr.end();
        
        while (first != last) {
            if (pred(first)) first = ctr.erase(first);
            else ++first;
        }
    }
}