# Takes list of dependencies as variadic argument.
macro(create_target target_name target_type major minor patch)
    # Get source files for target.
    file(GLOB_RECURSE sources CONFIGURE DEPENDS LIST_DIRECTORIES false "*.hpp" "*.cpp")
    list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}\\/tests\\/[0-z_]+\\.[ch]pp")

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
    include(utility)
    target_link_libraries_system(${target_name} ${ARGN})
    # Prevent linker language errors on header only libraries.
    set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)

    # Set version as preprocessor definitions.
    string(TOUPPER ${target_name} name_capitalized)
    target_compile_definitions(${target_name} PUBLIC "${name_capitalized}_VERSION_MAJOR=${major}")
    target_compile_definitions(${target_name} PUBLIC "${name_capitalized}_VERSION_MINOR=${minor}")
    target_compile_definitions(${target_name} PUBLIC "${name_capitalized}_VERSION_PATCH=${patch}")

    # Add tests if there is a test folder for this target and testing is enabled.
    if(NOT ${target_name} MATCHES "test_.+" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests" AND ${ENABLE_TESTING})
        include(CTest)
        enable_testing()

        # Get test files for target.
        file(GLOB_RECURSE tests CONFIGURE_DEPENDS LIST_DIRECTORIES false "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp")

        # Create a test for each source file.
        foreach(test IN ITEMS ${tests})
            get_filename_component(test_name ${test} NAME_WE)
            message(STATUS "Creating test ${test_name}.")

            # Create executable
            add_executable(test_${test_name} ${test})
            # Add dependencies.
            target_link_libraries(test_${test_name} PUBLIC ${target_name})
            # Prevent linker language errors on header only libraries.
            set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CXX)
        endforeach()
    endif()
endmacro()