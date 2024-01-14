#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/engine.hpp>

#include <boost/preprocessor.hpp>
#include <gtest/gtest.h>


namespace ve::testing {
    class engine_phased_test;


    namespace detail {
        /**
         * Data for keeping track of tests running within the context of the engine.
         * Contains the command line arguments (Set by main) and the current test (Set by engine_phased_test).
         * This class is not a service so that we can reset the entire state of the program between tests, without losing this information.
         */
        struct test_phase_data {
            static test_phase_data& instance(void) {
                static test_phase_data i { };
                return i;
            }

            std::vector<std::string> arguments = {};
            engine_phased_test* current = nullptr;
        };
    }


    /**
     * Base class for tests that run within the context of the engine.
     * Deriving classes will either override one of the [pre|post]_[init|loop|exit] methods and use the relevant VE_ENGINE_TEST_* macro,
     * or override multiple of these methods in a derived class, and then use the VE_MULTI_STAGE_ENGINE_TEST macro.
     */
    class engine_phased_test : public ::testing::Test {
    public:
        virtual ~engine_phased_test(void) = default;

        virtual void pre_init (void) {}
        virtual void post_init(void) {}
        virtual void pre_loop (void) {}
        virtual void post_loop(void) {}
        virtual void pre_exit (void) {}
        virtual void post_exit(void) {}
        virtual void dummy    (void) {}
    private:
        // GTest says not to do this but there's no real other way to integrate tests within the engine's lifecycle.
        // As long as we copy the rest of the body from GTests macros and just change the function name, it should be fine however.
        void TestBody(void) override {
            auto& test_data = ve::testing::detail::test_phase_data::instance();
            test_data.current = this;

            ve::services().clear();
            ve::get_service<ve::engine>().start(test_data.arguments);

            test_data.current = nullptr;
        }
    };
}


// Hijack game callbacks to allow tests to be executed within the engine lifecycle.
namespace ve {
    namespace game_callbacks {
        inline auto* get_test(void) { return ve::testing::detail::test_phase_data::instance().current; }

        void pre_init (void) { if (auto* test = get_test(); test) test->pre_init();  }
        void post_init(void) { if (auto* test = get_test(); test) test->post_init(); }
        void pre_loop (void) { if (auto* test = get_test(); test) test->pre_loop();  }
        void pre_exit (void) { if (auto* test = get_test(); test) test->pre_exit();  }
        void post_exit(void) { if (auto* test = get_test(); test) test->post_exit(); }

        void post_loop(void) {
            if (auto* test = get_test(); test) test->post_loop();
            get_service<engine>().stop();
        }
    }
}


// Program entry point during testing.
int main(int argc, char** argv) {
    auto& test_data = ve::testing::detail::test_phase_data::instance();
    test_data.arguments = { argv, argv + argc };

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


/** Returns the class name of the test with the given test name and test suite. */
#define VE_IMPL_GTEST_NAME(Suite, Test) BOOST_PP_SEQ_CAT((Suite)(_)(Test)(_Test))


/** 
 * Equivalent to TEST_F from GTest, but allows us to substitute in which method we wish to override.
 * Said method is then invoked through the TestBody of the parent class, after it starts the engine, which invokes the game_callbacks.
 * This is an ugly workaround and may break in the future, but there is currently no other way to integrate the tests into the engine's context,
 * while still running them within the GTest framework (or any other popular framework for that matter it seems).
 */
#define VE_IMPL_GTEST(Fixture, Suite, Test, Method)                                                                     \
static_assert(sizeof(#Fixture) > 1, "test_fixture_name must not be empty");                                             \
static_assert(sizeof(#Suite) > 1, "test_suite_name must not be empty");                                                 \
static_assert(sizeof(#Test) > 1, "test_name must not be empty");                                                        \
                                                                                                                        \
class VE_IMPL_GTEST_NAME(Suite, Test) : public Fixture {                                                                \
public:                                                                                                                 \
    VE_IMPL_GTEST_NAME(Suite, Test)() = default;                                                                        \
    ~VE_IMPL_GTEST_NAME(Suite, Test)() override = default;                                                              \
    VE_IMPL_GTEST_NAME(Suite, Test)(const VE_IMPL_GTEST_NAME(Suite, Test) &) = delete;                                  \
    VE_IMPL_GTEST_NAME(Suite, Test) &operator=(const VE_IMPL_GTEST_NAME(Suite, Test) &) = delete;                       \
    VE_IMPL_GTEST_NAME(Suite, Test)(VE_IMPL_GTEST_NAME(Suite, Test) &&) noexcept = delete;                              \
    VE_IMPL_GTEST_NAME(Suite, Test) &operator=(VE_IMPL_GTEST_NAME(Suite, Test) &&) noexcept = delete;                   \
private:                                                                                                                \
    void Method() override;                                                                                             \
    [[maybe_unused]] static ::testing::TestInfo *const test_info_;                                                      \
};                                                                                                                      \
                                                                                                                        \
::testing::TestInfo *const VE_IMPL_GTEST_NAME(Suite, Test)::test_info_ = ::testing::internal::MakeAndRegisterTestInfo(  \
    #Suite,                                                                                                             \
    #Test,                                                                                                              \
    nullptr,                                                                                                            \
    nullptr,                                                                                                            \
    ::testing::internal::CodeLocation(__FILE__, __LINE__),                                                              \
    (::testing::internal::GetTypeId<Fixture>()),                                                                        \
    ::testing::internal::SuiteApiResolver<Fixture>::GetSetUpCaseOrSuite(__FILE__, __LINE__),                            \
    ::testing::internal::SuiteApiResolver<Fixture>::GetTearDownCaseOrSuite(__FILE__, __LINE__),                         \
    new ::testing::internal::TestFactoryImpl<VE_IMPL_GTEST_NAME(Suite, Test)>                                           \
);                                                                                                                      \
                                                                                                                        \
void VE_IMPL_GTEST_NAME(Suite, Test)::Method()


#define VE_ENGINE_TEST_PRE_INIT(Suite, Test)  VE_IMPL_GTEST(ve::testing::engine_phased_test, Suite, Test, pre_init)
#define VE_ENGINE_TEST_POST_INIT(Suite, Test) VE_IMPL_GTEST(ve::testing::engine_phased_test, Suite, Test, post_init)
#define VE_ENGINE_TEST_PRE_LOOP(Suite, Test)  VE_IMPL_GTEST(ve::testing::engine_phased_test, Suite, Test, pre_loop)
#define VE_ENGINE_TEST_POST_LOOP(Suite, Test) VE_IMPL_GTEST(ve::testing::engine_phased_test, Suite, Test, post_loop)
#define VE_ENGINE_TEST_PRE_EXIT(Suite, Test)  VE_IMPL_GTEST(ve::testing::engine_phased_test, Suite, Test, pre_exit)
#define VE_ENGINE_TEST_POST_EXIT(Suite, Test) VE_IMPL_GTEST(ve::testing::engine_phased_test, Suite, Test, post_exit)
#define VE_MULTI_STAGE_ENGINE_TEST(Fixture, Suite, Test) VE_IMPL_GTEST(Fixture, Suite, Test, dummy) {}