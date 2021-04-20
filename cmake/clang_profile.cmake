function(set_clang_option option)
    # Clang-cl requires GCC-style options to be prefixed by /clang:
    if (${CMAKE_CXX_COMPILER} MATCHES ".+clang-cl.+")
        add_compile_options("/clang:${option}")
    else()
        add_compile_options(${option})
    endif()
endfunction()


# Note: this mutes some warnings that are produced by libraries because CONAN_SYSTEM_INCLUDES doesn't seem to work.
function(set_compiler_profile)
    add_compile_options(/EHsc)                  # TODO: Disable exceptions & RTTI.
    set_clang_option(-fbracket-depth=1024)
    set_clang_option(-ftemplate-backtrace-limit=0)

    # Enable extra warnings.
    set_clang_option(-Wall)
    set_clang_option(-Wextra)
    # set_clang_option(-Wpedantic)

    # Cast function pointer to void pointer. This is required for interacting with both GetProcAddress and dlsym.
    set_clang_option(-Wno-microsoft-cast)

    # Clang does not currently support [[no_unique_address]] on Windows (https://bugs.llvm.org/show_bug.cgi?id=50014).
    set_clang_option(-Wno-unknown-attributes)

    # Unused variables may exist when they are only used in the debug configuration.
    # e.g. storing the result of some call and using it with VE_ASSERT, when VE_ASSERT is disabled.
    set_clang_option(-Wno-unused-variable)
    set_clang_option(-Wno-unused-parameter)
    set_clang_option(-Wno-unused-private-field)
    set_clang_option(-Wno-unused-function)


    set_clang_option(-Wnon-virtual-dtor)
    set_clang_option(-Wcast-align)
    set_clang_option(-Woverloaded-virtual)
    set_clang_option(-Wformat=2)

    # TODO: Enable when the compiler supports it.
    # set_clang_option(-Wlifetime)
endfunction()