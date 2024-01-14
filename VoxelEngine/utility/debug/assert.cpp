#include <VoxelEngine/utility/debug/assert.hpp>
#include <VoxelEngine/utility/debug/intercept_assert.hpp>
#include <VoxelEngine/utility/services/logger.hpp>
#include <VoxelEngine/engine.hpp>

#include <SDL_messagebox.h>

#include <format>


namespace ve::debug::detail {
    void perform_assert(bool condition, std::string_view condition_string, std::string_view message, std::source_location where) {
        if (!condition) [[unlikely]] {
            const std::string description = std::format(
                "A VoxelEngine assertion failed. The engine will now exit.\n"
                "In file {}, at line {}: {}\n"
                "Condition evaluated to false: {}\n",
                where.file_name(),
                where.line(),
                message,
                condition_string
            );


            get_service<engine_logger>().fatal(description);


            auto make_intercepted_assertion = [&] {
                return intercepted_assertion {
                    std::string { condition_string },
                    std::string { message },
                    description,
                    where
                };
            };


            switch (get_assert_intercept_mode()) {
                case assert_intercept_mode::TERMINATE:
                    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "VoxelEngine Assertion Failure", description.c_str(), nullptr);
                    get_service<engine>().exit(EXIT_FAILURE, true);
                    break;
                case assert_intercept_mode::THROW:
                    throw make_intercepted_assertion();
                    break;
                case assert_intercept_mode::STORE:
                    detail::last_intercepted_assertion = make_intercepted_assertion();
                    break;
            }

        }
    }
}