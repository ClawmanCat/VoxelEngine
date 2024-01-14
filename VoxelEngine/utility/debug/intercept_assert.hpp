#pragma once

#include <VoxelEngine/core/core.hpp>

#include <string>
#include <source_location>
#include <optional>
#include <exception>


namespace ve::debug {
    /** Controls the behaviour of assertion failures. This is used for the engine's unit tests. */
    enum class assert_intercept_mode {
        /** Assertion failures will terminate the engine. This is the default behaviour. */
        TERMINATE,
        /** Assertions failures will throw a @ref intercepted_assertion. */
        THROW,
        /** Assertion failures will store a @ref intercepted_assertion, which is retrievable using @ref get_intercepted_assertion. */
        STORE
    };


    /** Stores data related to an assertion failure. */
    struct intercepted_assertion : std::exception {
        intercepted_assertion(void) = default;

        intercepted_assertion(std::string condition, std::string message, std::string description, std::source_location where) :
            std::exception(),
            condition(std::move(condition)),
            message(std::move(message)),
            description(std::move(description)),
            where(std::move(where))
        {}

        std::string condition;
        std::string message;
        std::string description;
        std::source_location where;
    };


    /** Sets the assertion intercept mode. See @ref assertion_intercept_mode. */
    extern void set_assert_interception_mode(assert_intercept_mode mode);
    /** Gets the current assertion intercept mode. */
    extern assert_intercept_mode get_assert_intercept_mode(void);
    /** If the current assertion intercept mode is STORE, and there was an intercepted assertion, returns and clears that intercepted assertion. */
    extern std::optional<intercepted_assertion> get_intercepted_assertion(void);


    namespace detail {
        extern assert_intercept_mode intercept_mode;
        extern std::optional<intercepted_assertion> last_intercepted_assertion;
    }
}