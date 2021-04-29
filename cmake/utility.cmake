# Add the given libraries as a dependency of the target, while setting their headers as system headers,
# preventing warnings from being produced from their headers.
# Note: this does not work with MSVC, msvc_profile.cmake handles muting these warnings for that compiler.
# The syntax for this function is the same as target_link_libraries; you may pass visibility flags (PRIVATE, PUBLIC, INTERFACE)
# for each library.
function(target_link_libraries_system target)
    set(visibility PRIVATE)


    foreach(lib IN ITEMS ${ARGN})
        # Library names could be prefixed by PUBLIC / PRIVATE / INTERFACE
        if (lib MATCHES "PUBLIC|PRIVATE|INTERFACE")
            set(visibility ${lib})
            continue()
        endif()


        # Link the libraries as SYSTEM to mute warnings
        get_target_property(include_dirs ${lib} INTERFACE_INCLUDE_DIRECTORIES)
        target_include_directories(${target} SYSTEM ${visibility} ${include_dirs})
        target_link_libraries(${target} ${lib})


        # Restore default visibility in case the next item has no specifier.
        set(visibility PRIVATE)
    endforeach()
endfunction()


# Run the given python script from the tools directory with the given arguments (ARGN).
macro(run_python name)
    find_package(PythonInterp REQUIRED)

    # arg;arg;arg => arg arg arg
    foreach(arg IN ITEMS ${ARGN})
        set(args ${args} ${arg})
    endforeach()

    message(STATUS "Running python script ${name}...")

    execute_process(
            COMMAND "${PYTHON_EXECUTABLE}" "${CMAKE_SOURCE_DIR}/tools/${name}.py" ${args}
            RESULT_VARIABLE py_result
    )

    if (NOT "${py_result}" STREQUAL "0")
        message(SEND_ERROR "Script ${name} failed: ${py_result}")
    endif()
endmacro()


# Read a file containing a space-separated key value pair on each line into a list of pairs,
# then invokes fn(k, v) for each pair.
function(parse_kv_file path fn)
    file(READ ${path} file_data)
    string(REPLACE "\n" "${Esc};" file_data ${file_data})

    # Parse name / URL pairs and add remotes.
    foreach(kv IN ITEMS ${file_data})
        separate_arguments(kv)

        list(GET kv 0 k)
        list(GET kv 1 v)

        cmake_language(CALL ${fn} ${k} ${v})
    endforeach()
endfunction()