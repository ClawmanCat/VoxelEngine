#pragma once

#include <VoxelEngine/core/core.hpp>

#include <source_location>
#include <string>
#include <string_view>
#include <format>


namespace ve::debug::detail {
    /**
     * Asserts that the given condition is true, and triggers an assertion failure if it is not.
     * When an assertion failure is triggered:
     *    - The failure will be logged to the engine logger.
     *    - A dialog window with the assertion failure will be shown which will block the calling thread until it is closed.
     *    - Closing the dialog window will exit the engine through ve::engine::exit.
     * @param condition The condition which will be checked.
     * @param condition_string A stringified version of the condition.
     * @param message An optional message to display alongside the condition string in case of assertion failure.
     * @param where The source location where the assert was performed.
     */
    extern void perform_assert(
        bool condition,
        std::string_view condition_string,
        std::string_view message = "",
        std::source_location where = std::source_location::current()
    );


    /** Wrapper around @ref perform_assert to elude processing of the error string in the case where the condition is true. */
    template <typename... Args> inline void perform_assert_wrapper(
        bool condition,
        std::string_view condition_string,
        std::source_location where,
        std::format_string<Args...> message_fmt = "",
        Args&&... message_args
    ) {
        if (condition) [[unlikely]] {
            std::string message_string = std::format(std::move(message_fmt), fwd(message_args)...);
            perform_assert(condition, condition_string, message_string, where);
        }
    }
}


#define VE_ASSERT(Condition, ...) \
ve::debug::detail::perform_assert_wrapper(Condition, #Condition, std::source_location::current(), __VA_ARGS__)

#ifdef VE_DEBUG
    #define VE_DEBUG_ASSERT(Condition, ...) VE_ASSERT(Condition, __VA_ARGS__)
#else
    #define VE_DEBUG_ASSERT(Condition, ...)
#endif