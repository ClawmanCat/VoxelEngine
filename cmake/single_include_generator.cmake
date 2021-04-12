# Generates a header with the given name in directory which includes all other headers in that directory.
# Optional arguments can be used to pass headers to be excluded from the list.
function(generate_si_header target directory header_name)
    set(target_path "${CMAKE_SOURCE_DIR}/${target}/${directory}/${header_name}")
    message(STATUS "Updating auto-generated header list at ${target_path}")

    file(GLOB_RECURSE headers "${CMAKE_SOURCE_DIR}/${target}/${directory}/*.hpp")
    list(REMOVE_ITEM headers ${target_path})

    foreach(arg IN ITEMS ${ARGN})
        list(FILTER headers EXCLUDE REGEX ".*${arg}")
    endforeach()

    file(
        WRITE
        ${target_path}
        "// This file is automatically generated by CMake.\n"
        "// Do not edit it, as your changes will be overwritten the next time CMake is run.\n"
        "// This file includes all headers in ${target}/${directory}.\n\n"
        "#pragma once\n\n"
    )

    foreach(header IN ITEMS ${headers})
        file(RELATIVE_PATH header_path "${CMAKE_SOURCE_DIR}" "${header}")
        file(APPEND ${target_path} "#include <${header_path}>\n")
    endforeach()
endfunction()