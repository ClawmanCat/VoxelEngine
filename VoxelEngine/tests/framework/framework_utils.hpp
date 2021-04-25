#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/overloadable_setting.hpp>


namespace ve::tests {
    // Change these settings by overloading overloadable_test_settings<overloaded_settings_tag>
    template <typename> struct overloadable_test_settings {
        constexpr static inline u64 num_ticks = 1;
    };
    
    using test_settings = overloadable_test_settings<overloaded_settings_tag>;
    
    
    // Can be used to show a progress message while executing long running tests.
    struct progress_indicator {
        std::string_view message;
        std::size_t task_count;
        std::size_t interval = task_count / 10;
        std::size_t progress = 0;
        
        void update(void) {
            ++progress;
            
            if (progress % interval == 0) {
                VE_LOG_INFO(""s + message + " [" + format(progress) + " / " + format(task_count) + "]");
            }
        }
        
        static std::string format(std::size_t num) {
            std::size_t prefix = num;
            char suffix = '?';
            
            auto format_case = [&](auto factor, char ch) {
                if (num >= factor) {
                    prefix = num / factor;
                    suffix = ch;
                }
            };
            
            format_case((u64) 1e3, 'K');
            format_case((u64) 1e6, 'M');
            format_case((u64) 1e9, 'B');
            
            return (suffix == '?')
                   ? std::to_string(prefix)
                   : std::to_string(prefix) + suffix;
        }
    };
}