#include <VoxelEngine/tests/define_game_symbols.hpp>
#include <VoxelEngine/event/priority_storage.hpp>
#include <VoxelEngine/utils/priority.hpp>

#include <range/v3/view/zip.hpp>

#include <string>
#include <array>
#include <utility>
#include <iostream>
#include <algorithm>
#include <random>
#include <string_view>

using namespace ve;
using namespace ve::events;


int main() {
    const std::array values = {
        std::pair { "LOWEST_01",  priority::LOWEST  },
        std::pair { "LOWEST_02",  priority::LOWEST  },
        std::pair { "LOWEST_03",  priority::LOWEST  },
        std::pair { "LOW_01",     priority::LOW     },
        std::pair { "LOW_02",     priority::LOW     },
        std::pair { "LOW_03",     priority::LOW     },
        std::pair { "NORMAL_01",  priority::NORMAL  },
        std::pair { "NORMAL_02",  priority::NORMAL  },
        std::pair { "NORMAL_03",  priority::NORMAL  },
        std::pair { "HIGH_01",    priority::HIGH    },
        std::pair { "HIGH_02",    priority::HIGH    },
        std::pair { "HIGH_03",    priority::HIGH    },
        std::pair { "HIGHEST_01", priority::HIGHEST },
        std::pair { "HIGHEST_02", priority::HIGHEST },
        std::pair { "HIGHEST_03", priority::HIGHEST }
    };
    
    
    // Make a randomly sorted copy of values.
    auto values_copy = values;
    
    std::random_device dev;
    std::default_random_engine gen(dev());
    
    std::shuffle(values_copy.begin(), values_copy.end(), gen);
    
    
    // Insert random ordered elements into storage.
    priority_storage<std::string> storage;
    for (auto& [val, p] : values_copy) storage.insert(val, p);
    
    
    // Assert that storage is ordered.
    auto get_priority = [](std::string_view str) { str.remove_suffix(3); return str; };
    
    for (const auto& [value, target] : ranges::views::zip(storage, values)) {
        if (get_priority(value) != get_priority(target.first)) return -1;
    }
    
    
    return 0;
}