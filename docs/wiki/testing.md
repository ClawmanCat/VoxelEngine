# Testing VoxelEngine
Each subproject within the project can provide its own tests by adding .cpp files in the `<Subproject>/tests` folder.  
Each cpp file in this folder will automatically be converted to a standalone CTest executable. Tests are only generated if CMake is run with the setting `VE_ENABLE_TESTING=ON`.  
The engine uses [Google Test](https://github.com/google/googletest) as its testing framework, but tests can still be generated without using this framework, as long as they provide a suitable main method.


### Engine Integration
It is possible to run tests as standalone as well as within the context of the engine.  
For standalone tests, the default GTest test macros can be used. For engine tests, `VE_ENGINE_TEST_*` macros are provided (See `VoxelEngine/testing/test_framework.hpp`), allowing tests to run during a specific phase of the engine's lifecycle:

```c++
#include <VoxelEngine/testing/test_common.hpp>
#include <VoxelEngine/utility/services/logger.hpp>


TEST(MySuite, MyStandaloneTest) {
    ve::get_service<ve::engine_logger>() << "This is a standalone test." << std::endl;    
}


VE_ENGINE_TEST_PRE_INIT(MySuite, EnginePreInitTest) {
    ve::get_service<ve::engine_logger>() << "This test runs before the engine is initialized." << std::endl;  
}


VE_ENGINE_TEST_PRE_LOOP(MySuite, EnginePreLoopTest) {
    ve::get_service<ve::engine_logger>() << "This test runs before the engine main loop." << std::endl;  
}
```

By default, the entire engine lifecycle is traversed for each test. That means, for example, that if there are two PRE_LOOP tests, the engine will first start, then run the first test, stop, then start again, run the second test and then stop again.  
If it is required to run several tests within one invocation of the engine's lifecycle, create a class inheriting from `ve::testing::engine_phased_test` and use the `VE_MULTI_STAGE_ENGINE_TEST` macro:

```c++
#include <VoxelEngine/testing/test_common.hpp>
#include <VoxelEngine/utility/services/logger.hpp>


struct multi_test : ve::testing::engine_phased_test {
    void post_init(void) override {
        ve::get_service<ve::engine_logger>() << "This part of the test runs after the engine has initialized." << std::endl;  
    }
    
    void pre_loop(void) override {
        ve::get_service<ve::engine_logger>() << "This part of the test runs before the engine main loop." << std::endl;  
    }
    
    void post_loop(void) override {
        ve::get_service<ve::engine_logger>() << "This part of the test runs after the engine main loop." << std::endl;  
    }
    
    void pre_exit(void) override {
        ve::get_service<ve::engine_logger>() << "This part of the test runs before the engine exits." << std::endl;  
    }
};


VE_MULTI_STAGE_ENGINE_TEST(multi_test, MySuite, MyMultiStageTest)
```

Note that during testing the engine always exits automatically after looping once. It is not required to manually call `engine::stop` from test code.  
Engine tests are handled as if they were a game, meaning that game callbacks must be provided for the engine to invoke. This is handled automatically when including `VoxelEngine/testing/test_common.hpp`. Tests that do not include this header must define their own callbacks to prevent missing symbol errors during link time.


### Testing Utilities
The engine provides several testing utilities to make testing easier. These are automatically included when including `VoxelEngine/testing/test_common.hpp`.

###### Type Equality Macros
For testing the template metaprogramming sections of the engine, macros equivalent to GTest's `[ASSERT|EXPECT]_[EQ|NE]` are provided to compare types rather than values:

```c++
#include <VoxelEngine/testing/test_common.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


TEST(MySuite, AreTypesEqual) {
    ASSERT_TYPE_EQ(typename ve::meta::pack<int, char, bool>::head, int);
    ASSERT_TYPE_NE(typename ve::meta::pack<int, char, bool>::head, void);
}
```

###### Regex Matching Macros
Replacement macros are provided for GMock's regex matchers, as those do not implement all regex features:

```c++
#include <VoxelEngine/testing/test_common.hpp>
#include <VoxelEngine/utility/meta/pack/pack.hpp>


TEST(MySuite, DoesStringMatch) {
    ASSERT_MATCHES(
        "Katternije says Meow!",
        R"RGX(\w+ says ((Awoo)|(Bork)|(Meow)[\.\!\?]?))RGX"
    );
}
```

###### Fuzz Testing
The engine provides some basic infrastructure to perform fuzz testing through the `ve::testing::operation_fuzzer` class.
This class can both invoke functions with fuzzed values, as well as fuzz the execution of functions themselves:

```c++
#include <VoxelEngine/testing/test_common.hpp>


namespace test     = ve::testing;
namespace fuzzargs = ve::testing::fuzz_arguments;


TEST(MySuite, CatContainerFuzzTest) {
    struct state {
        cat_container container;
        ve::hash_set<std::string> expected_state; 
    };
    
    
    test::operation_fuzzer<state> fuzzer;
    
    // Operation to add an item to the container:
    fuzzer.add_operation(
        // Operation to invoke, must accept state and extra arguments passed below:
        [] (state& s, const std::string& random_name) {
            if (s.expected_state.contains(random_name)) {
                EXPECT_FALSE(s.container.make_cat(random_name));
            } else {
                EXPECT_TRUE(s.container.make_cat(random_name));
                s.expected_state.insert(random_name);
            }
        },
        // Weight for how often to call this operation compared to other ones:
        2.0f,
        // Fuzzed data to pass to the operation. Can be any class that is invocable with the correct return type:
        fuzzargs::random_string {
            4, 4,
            // Any uppercase letter or a space.
            fuzzargs::random_string::charset_alpha_upper + " "
        }
    );
    
    // Operation to remove an item from the container:
    fuzzer.add_operation(
        [] (state& s, const std::string& random_name) {
            if (s.expected_state.contains(random_name)) {
                EXPECT_TRUE(s.container.remove_cat(random_name));
                s.expected_state.erase(random_name);
            } else {
                EXPECT_FALSE(s.container.remove_cat(random_name));
            }
        },
        1.0f,
        fuzzargs::random_string {
            4, 4,
            fuzzargs::random_string::charset_alpha_upper + " "
        }
    );
    
    // Randomly selects from the provided operations and runs them N times.
    fuzzer.invoke(
        // Initial state:
        state { },
        // How many operations to run:
        1e6,
        // How many threads to run operations on (for testing thread-safe objects):
        1
    );
}
```