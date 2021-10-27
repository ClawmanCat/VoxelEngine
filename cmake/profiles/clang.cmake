function(load_compiler_profile)
    include(profiles/gcclike_common)
    include(utility)

    load_common_profile()


    get_platform_name(os)

    if (${os} STREQUAL windows)
        # Clang does not support [[no_unique_address]] on Windows.
        # (https://bugs.llvm.org/show_bug.cgi?id=50014)
        set_compiler_option(-Wno-unknown-attributes)

        if (${CMAKE_CXX_COMPILER} MATCHES ".+clang-cl.+")
            add_compile_options(/EHsc)
        endif()
    endif()
endfunction()