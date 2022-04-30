# Creates a target in the current directory with the given name and type (EXECUTABLE, INTERFACE, etc.).
# A list of targets can be provided as variadic parameters to pass a list of dependencies for the target.
function(create_target name type major minor patch)
    file(GLOB_RECURSE sources CONFIGURE DEPENDS LIST_DIRECTORIES false "*.cpp" "*.hpp")
    list(FILTER sources EXCLUDE REGEX "${CMAKE_CURRENT_SOURCE_DIR}\\/tests\\/.*") # Test sources are not part of the target itself.

    create_target_from_sources(${name} ${type} ${major} ${minor} ${patch} "${sources}" ${ARGN})
endfunction()


# Equivalent to create_target, but a list of sources is explicitly provided.
function(create_target_from_sources name type major minor patch sources)
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

    # Add preprocessor definition to manage dllexport / dllimport.
    target_compile_definitions(${name} PRIVATE "BUILDING_${name_capitalized}=1")


    # A target may contain a test folder. Each cpp file in such a folder should be turned into a new target.
    # (But only if ENABLE_TESTING is set.)
    if (${ENABLE_TESTING})
        create_tests(${name})
    endif()


    # On Windows, filter what symbols are exported if this is a shared library and we have a filter file.
    get_platform_name(os)

    if (${os} STREQUAL "windows" AND ${type} STREQUAL "SHARED" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/symgen.filter")
        create_symbol_filter(${name} "${CMAKE_CURRENT_SOURCE_DIR}/symgen.filter")
    endif()
endfunction()


function (create_tests parent_target)
    # Tests cannot have nested tests. Only add tests if the current target is itself not a test.
    # (Note: test names are guaranteed to start with test_)
    if (NOT ${parent_target} MATCHES "test_.+" AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
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
            target_link_libraries(test_${test_name} PUBLIC ${parent_target})

            # Prevent linker language errors on header only libraries.
            set_target_properties(test_${test_name} PROPERTIES LINKER_LANGUAGE CXX)
        endforeach()
    endif()
endfunction()


# Creates a filter for what symbols will be added to the symbol table of the target DLL.
# Symbol filtering is performed using the SymbolGenerator program (https://github.com/ClawmanCat/SymbolGenerator),
# and used to create a .def file. "argument_file" should be the path of a file containing the command line arguments for SymbolGenerator.
# Note: this function is only meant for Windows platforms as it is meant to reduce the symbol count to below the 64K DLL limit.
function(create_symbol_filter target argument_file)
    create_symbol_filter_impl(
        ${target}
        ${target}
        ${target}
        ${argument_file}
        "${PROJECT_BINARY_DIR}/${target}/CMakeFiles/${target}.dir"
    )
endfunction()


# Equivalent to above, but with some extra options:
# attach_to_target: SymbolGenerator will be invoked while building this target.
# lib_target:       The name of the DLL that will use the DEF file.
# output_filename:  The name of the output file.
# src_dir:          Where to look for .obj files.
# This version of create_symbol_filter is provided for the case where a DLL depends on a CMake OBJECT target,
# and should export all symbols from that target itself.
function(create_symbol_filter_impl attach_to_target lib_target output_filename argument_file src_dir)
    get_platform_name(os)
    if (NOT ${OS} STREQUAL "windows")
        message(FATAL "create_symbol_filter can only be used on Windows platforms (other platforms do not require symbol filtering).")
    endif()

    set(symgen_url  "https://github.com/ClawmanCat/SymbolGenerator/releases/latest/download/SymbolGenerator.exe")
    set(symgen_path "${CMAKE_SOURCE_DIR}/tools/SymbolGenerator.exe")

    if (NOT EXISTS "${symgen_path}")
        message(STATUS "Failed to find SymbolGenerator.exe. Downloading latest release...")
        file(DOWNLOAD "${symgen_url}" "${symgen_path}")
    endif()

    file(READ ${argument_file} args)
    string(REPLACE "\n" " " args ${args})
    string(REPLACE " "  ";" args ${args})

    add_custom_command(
            TARGET ${attach_to_target}
            PRE_LINK
            COMMAND SymbolGenerator
            ARGS
                ${args}
                -i "${src_dir}"
                -o "${PROJECT_BINARY_DIR}/${output_filename}.def"
                --lib ${lib_target}
                --cache
            VERBATIM
            WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/tools"
    )

    # Since this is Windows-only, we can assume a MSVC-like command line (MSVC or Clang-CL).
    target_link_options(${attach_to_target} PRIVATE "/DEF:${output_filename}.def")
endfunction()