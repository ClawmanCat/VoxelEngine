# Creates a target in the current directory with the given name and type (EXECUTABLE, INTERFACE, etc.).
# A list of targets can be provided as variadic parameters to pass a list of dependencies for the target.
function(create_target name type major minor patch)
    create_target_filtered(${name} ${type} ${major} ${minor} ${patch} "__NONEXISTENT_FILENAME__" ${ARGN})
endfunction()


# Equivalent to the above function, but a regex filter can be provided to prevent
# the inclusion of certain source files from the target
function(create_target_filtered name type major minor patch filter)
    include(utility)


    # Get source files for target.
    file(GLOB_RECURSE sources CONFIGURE DEPENDS LIST_DIRECTORIES false "*.cpp" "*.hpp")
    list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}\\/tests\\/.*")
    list(FILTER sources EXCLUDE REGEX ${filter})


    # Create target of correct type and add sources.
    if (${type} STREQUAL "EXECUTABLE")
        add_executable(${name} ${sources})
    elseif (${type} STREQUAL "INTERFACE")
        add_library(${name} ${type})
        target_sources(${sources})
    else ()
        add_library(${name} ${type} ${sources})
    endif()


    # Add dependencies.
    target_link_libraries_system(${name} ${ARGN})


    # Specifying the linker language manually is required for header only libraries,
    # since the language cannot be deduced from the file extension.
    set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)


    # Add preprocessor definitions for the target version.
    string(TOUPPER ${name} name_capitalized)
    target_compile_definitions(${name} PUBLIC "${name_capitalized}_VERSION_MAJOR=${major}")
    target_compile_definitions(${name} PUBLIC "${name_capitalized}_VERSION_MINOR=${minor}")
    target_compile_definitions(${name} PUBLIC "${name_capitalized}_VERSION_PATCH=${patch}")


    # A target may contain a test folder. Each cpp file in such a folder should be turned into a new target.
    # (But only if ENABLE_TESTING is set.)
    if (${ENABLE_TESTING})
        # Tests cannot have nested tests. Only add tests if the current target is itself not a test.
        # (Note: test names are guaranteed to start with test_)
        if (NOT ${name} MATCHES "test_.+" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
            # Get test files for target.
            file(GLOB_RECURSE tests CONFIGURE_DEPENDS LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")


            # Create a test for each source file.
            foreach(test IN ITEMS ${tests})
                get_filename_component(test_name ${test} NAME_WE)
                message(STATUS "Creating test ${test_name}.")

                # Create executable
                add_executable(test_${test_name} ${test})
                add_test(
                    NAME ${test_name}
                    COMMAND test_${test_name}
                    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
                )

                # Add dependencies.
                target_link_libraries(test_${test_name} PUBLIC ${name})

                # Prevent linker language errors on header only libraries.
                set_target_properties(${name} PROPERTIES LINKER_LANGUAGE CXX)
            endforeach()
        endif()
    endif()
endfunction()