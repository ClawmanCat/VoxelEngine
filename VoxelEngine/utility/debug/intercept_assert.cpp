#include <VoxelEngine/utility/debug/intercept_assert.hpp>


namespace ve::debug {
    namespace detail {
        assert_intercept_mode intercept_mode = assert_intercept_mode::TERMINATE;
        std::optional<intercepted_assertion> last_intercepted_assertion = std::nullopt;
    }


    void set_assert_interception_mode(assert_intercept_mode mode) {
        detail::intercept_mode = mode;
    }


    assert_intercept_mode get_assert_intercept_mode(void) {
        return detail::intercept_mode;
    }


    std::optional<intercepted_assertion> get_intercepted_assertion(void) {
        return detail::last_intercepted_assertion;
    }
}