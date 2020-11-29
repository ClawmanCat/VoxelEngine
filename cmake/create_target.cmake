# Takes list of dependencies as variadic argument.
macro(create_target target_name target_type major minor patch)
    # Get source files for target.
    file(GLOB_RECURSE sources CONFIGURE DEPENDS LIST_DIRECTORIES false "*.hpp" "*.cpp")
    list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}\\/tests\\/[0-z_]+\\.[ch]pp")
    message(STATUS ${sources})

    # Create target of correct type and add sources.
    if (${target_type} STREQUAL "EXECUTABLE")
        add_executable(${target_name} ${sources})
    elseif (${target_type} STREQUAL "INTERFACE")
        add_library(${target_name} ${target_type})
        target_sources(${sources})
    else ()
        add_library(${target_name} ${target_type} ${sources})
    endif()

    # Add dependencies.
    target_link_libraries(${target_name} ${ARGN})
    # Prevent linker language errors on header only libraries.
    set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)

    # Set version as preprocessor definitions.
    string(TOUPPER ${target_name} name_capitalized)
    target_compile_definitions(${target_name} PUBLIC "${name_capitalized}_VERSION_MAJOR=${major}")
    target_compile_definitions(${target_name} PUBLIC "${name_capitalized}_VERSION_MINOR=${minor}")
    target_compile_definitions(${target_name} PUBLIC "${name_capitalized}_VERSION_PATCH=${patch}")

    # Add tests if there is a test folder for this target and testing is enabled.
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests" AND DEFINED ENABLE_TESTING)
        set(STOP_UNUSED_VAR_WARNING ${ENABLE_TESTING})

        include(CTest)
        enable_testing()

        # Get test files for target.
        file(GLOB_RECURSE tests CONFIGURE_DEPENDS LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

        # Create a test for each source file.
        foreach(test IN ITEMS ${tests})
            get_filename_component(test_name ${test} NAME_WE)

            add_executable("test_${test_name}" ${test})
            add_test(NAME "test_${test_name}" COMMAND "test_${test_name}")

            target_include_directories("test_${test_name}" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/tests")
            set_target_properties("test_${test_name}" PROPERTIES LINKER_LANGUAGE CXX)

            # Make the test have the target as a dependency.
            target_link_libraries("test_${test_name}" PUBLIC ${target_name})
            target_compile_definitions("test_${test_name}" PUBLIC ${name_capitalized}_TESTING)
        endforeach()
    endif()
endmacro()