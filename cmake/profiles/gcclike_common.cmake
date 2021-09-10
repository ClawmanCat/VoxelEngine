function(set_compiler_option option)
    # Clang-CL requires GCC-style options be prefixed by /clang:
    if (${CMAKE_CXX_COMPILER} MATCHES ".+clang-cl.+")
        add_compile_options("/clang:${option}")
    else()
        add_compile_options(${option})
    endif()
endfunction()


# Large parts of the profile of GCC and Clang are the same,
# This secondary profile can be loaded by both the GCC and Clang profiles to reduce redundancy.
function(load_common_profile)
    # Enable extra warnings.
    set_compiler_option(-Wall)
    set_compiler_option(-Wextra)
    set_compiler_option(-Wpedantic)

    set_compiler_option(-Wnon-virtual-dtor)
    set_compiler_option(-Wcast-align)
    set_compiler_option(-Woverloaded-virtual)
    set_compiler_option(-Wformat=2)


    # While it is technically UB to cast between void* and function pointer,
    # most operating systems require it to dynamically load symbols through GetProcAddress / dlsym.
    set_compiler_option(-Wno-microsoft-cast)

    # This warning is obsolete, as the standard does not forbid this anymore in C++20.
    # (http://eel.is/c++draft/cpp.replace.general#15)
    set_compiler_option(-Wno-gnu-zero-variadic-macro-arguments)

    # Macro-expansion may result in superfluous semicolons.
    # In some cases, the extra code required to prevent this would make things more unreadable.
    set_compiler_option(-Wno-extra-semi)

    # Enabled to allow usage of compound literals.
    # This is supported by all major compilers, even MSVC, which doesn't fully implement C99.
    set_compiler_option(-Wno-c99-extensions)

    # Disable warnings for unused code.
    set_compiler_option(-Wno-unused-variable)
    set_compiler_option(-Wno-unused-parameter)
    set_compiler_option(-Wno-unused-private-field)
    set_compiler_option(-Wno-unused-function)

    # Usage of magic_enum requires a large recursion depth.
    set_compiler_option(-fbracket-depth=1024)
endfunction()