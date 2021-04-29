#pragma once

#include <VoxelEngine/core/core.hpp>
#include <VoxelEngine/utility/logger.hpp>
#include <VoxelEngine/utility/traits/string_arg.hpp>

#include <magic_enum.hpp>
#include <boost/preprocessor.hpp>


namespace ve::tests {
    template <meta::string_arg Name> struct test {};
    
    
    struct test_fn_container {
        // Only one of these will be filled per test.
        Fn<void> test_without_result = nullptr;
        Fn<i32> test_with_result = nullptr;
        
        std::string_view name = "Unnamed Test";
        
        
        test_fn_container(void) = default;
        test_fn_container(Fn<void> fn) : test_without_result(fn) {}
        test_fn_container(Fn<i32> fn) : test_with_result(fn) {}
    };
    
    
    // Lambda requires explicit cast to function, so wrap it and perform the cast.
    template <meta::string_arg Name> struct fn_conversion_wrapper {
        fn_conversion_wrapper(auto fn) {
            if constexpr (std::is_invocable_r_v<i32, decltype(fn)>) ctr.test_with_result = (Fn<i32>) fn;
            else ctr.test_without_result = (Fn<void>) fn;
            
            ctr.name = Name.c_string;
        }
        
        operator test_fn_container(void) const { return ctr; }
        
        test_fn_container ctr;
    };
    
    
    // Store tests to be ran later.
    class test_registry {
    public:
        enum testing_stage { PRE_INIT, POST_INIT, PRE_LOOP, POST_LOOP, PRE_EXIT, POST_EXIT };
        
        
        static test_registry& instance(void) {
            static test_registry i;
            return i;
        }
        
        
        void add_test(test_fn_container test, testing_stage stage) {
            tests[stage].push_back(test);
        }
        
        
        bool run_tests(testing_stage stage) {
            bool success = true;
            
            for (auto test : tests[stage]) {
                VE_LOG_INFO("Running test "s + test.name + "...");
                
                if (test.test_with_result) {
                    i32 result = test.test_with_result();
                    
                    if (result) {
                        VE_LOG_ERROR("Test "s + test.name + " failed with exit code " + std::to_string(result));
                        ++failed_tests;
                        success = false;
                    } else {
                        ++successful_tests;
                    }
                } else {
                    // If we just want to test if a piece of code compiles or that it doesn't crash,
                    // we don't neccesarily need to return a value.
                    test.test_without_result();
                    ++successful_tests;
                }
            }
            
            return success;
        }
        
        
        void print_stats(void) const {
            if (failed_tests == 0) {
                VE_LOG_INFO("All "s + std::to_string(successful_tests) + " completed successfully.");
            } else {
                VE_LOG_WARN(
                    "Tests finished with errors: "s +
                    std::to_string(failed_tests) +
                    " out of " +
                    std::to_string(failed_tests + successful_tests) +
                    " failed."
                );
            }
        }
        
        
        bool has_errors(void) const {
            return failed_tests > 0;
        }
    private:
        std::array<
            std::vector<test_fn_container>,
            magic_enum::enum_count<testing_stage>()
        > tests;
        
        std::size_t successful_tests = 0, failed_tests = 0;
    };
}


// Generates a test function to be ran during the execution of the test program at the given stage.
// Usage: VE_TEST(my_test, PRE_INIT) { test body... }
// Stage must be a member of ve::tests::test_registry::testing_stage.
#define VE_TEST(test_name, stage)                                                           \
template <> struct ve::tests::test<#test_name> {                                            \
    constexpr static std::string_view name = #test_name;                                    \
    static test_fn_container test_impl;                                                     \
                                                                                            \
    const static inline ve::meta::null_type ve_impl_register_test = [](){                   \
        ve::tests::test_registry::instance().add_test(                                      \
            ve::tests::test<#test_name>::test_impl,                                         \
            ve::tests::test_registry::stage                                                 \
        );                                                                                  \
        return ve::meta::null_type { };                                                     \
    }();                                                                                    \
};                                                                                          \
                                                                                            \
ve::tests::test_fn_container ve::tests::test<#test_name>::test_impl                         \
= (fn_conversion_wrapper<#test_name>) []() /* Test body will be appended here. */


// Wrapper to force expansion of the arguments to VE_TEST
#define VE_IMPL_TEST_WITH_TEMPLARGS_MAKE_TEST(name, stage)                                  \
VE_TEST(name, stage)


// Given a tuple data = (test_name, test_stage) and a tuple of template arguments Elem,
// generate a VE_TEST which calls test_name with the given arguments.
#define VE_IMPL_TEST_WITH_TEMPLARGS_MACRO(Rep, Data, Elem)                                  \
VE_IMPL_TEST_WITH_TEMPLARGS_MAKE_TEST(                                                      \
    BOOST_PP_SEQ_CAT((BOOST_PP_TUPLE_ELEM(0, Data))(_)(Rep)),                               \
    BOOST_PP_TUPLE_ELEM(1, Data)                                                            \
) {                                                                                         \
    return BOOST_PP_TUPLE_ELEM(0, Data)<                                                    \
        BOOST_PP_TUPLE_ENUM(Elem)                                                           \
    >();                                                                                    \
};


// Given a sequence of template argument combinations, generates VE_TESTs that call test_name
// with every combination of arguments.
#define VE_TEST_WITH_TEMPLARGS(test_name, stage, seq)                                       \
BOOST_PP_SEQ_FOR_EACH(                                                                      \
    VE_IMPL_TEST_WITH_TEMPLARGS_MACRO,                                                      \
    (test_name, stage),                                                                     \
    seq                                                                                     \
);