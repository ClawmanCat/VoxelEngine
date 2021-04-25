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