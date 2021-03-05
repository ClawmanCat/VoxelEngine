function(set_compiler_profile)
    add_compile_options(/EHsc)
    add_compile_options(/clang:-fbracket-depth=1024)
    add_compile_options(/clang:-Wall /clang:-Wextra)      # -Wpedantic will trigger ~4000 warnings in library code.
    add_compile_options(/clang:-Wno-unknown-attributes)   # Mute warnings due to lack of C++20 attribute support. (Remove for Clang 12)
    add_compile_options(/clang:-Wno-microsoft-cast)       # Cast function pointer to void pointer.

    # Unused variables / functions may exist when they are only used in the debug configuration.
    # e.g. storing the result of some call and using it with VE_ASSERT.
    # We still want to get these warnings if we're in debug mode however.
    if (NOT CMAKE_BUILD_TYPE STREQUAL "DEBUG")
        add_compile_options(/clang:-Wno-unused-variable)
        add_compile_options(/clang:-Wno-unused-function)
        add_compile_options(/clang:-Wno-unused-private-field)
    endif()

    # Warnings from library headers
    add_compile_options(/clang:-Wno-deprecated-volatile)
    add_compile_options(/clang:-Wno-unused-parameter)
endfunction()