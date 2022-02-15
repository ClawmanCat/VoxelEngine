# Parse a file consisting of key-value pairs, where each pair is separated by a newline,
# and each key and value are separated by a space.
# fn(k, v) is executed for each pair.
macro(parse_kv_file path fn)
    file(READ ${path} file_data)
    string(REPLACE "\n" "${Esc};" file_data ${file_data})


    foreach(kv IN ITEMS ${file_data})
        separate_arguments(kv)

        list(GET kv 0 k)
        list(GET kv 1 v)

        cmake_language(CALL ${fn} ${k} ${v})
    endforeach()
endmacro()


# Returns a full lowercase string containing the name of the current operating system.
function(get_platform_name return_var)
    string(TOLOWER ${CMAKE_SYSTEM_NAME} result)

    if (${result} STREQUAL "darwin")
        set(result "osx")
    endif()

    set(${return_var} ${result} PARENT_SCOPE)
endfunction()


# Returns a full lowercase string containing the name of the current compiler.
function (get_compiler_name return_var)
    string(TOLOWER ${CMAKE_CXX_COMPILER_ID} result)

    if (${result} STREQUAL "gnu")
        set(result "gcc")
    endif()

    set(${return_var} ${result} PARENT_SCOPE)
endfunction()


# Equivalent to target_link_libraries, but links the libraries as system libraries,
# preventing them from producing warnings.
# Library names are passed as variadic parameters, optionally prefixed by PUBLIC, PRIVATE or INTERFACE.
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


        # MSVC requires the /external:I... option be used to disable warnings in library code.
        # Note: this also requires /experimental:external and /external:W0 are set.
        get_compiler_name(compiler)

        if (${compiler} STREQUAL msvc)
            foreach(${dir} IN ITEMS ${include_dirs})
                add_compile_options(/external:I${dir})
            endforeach()
        endif()


        # Restore default visibility in case the next item has no specifier.
        set(visibility PRIVATE)
    endforeach()
endfunction()