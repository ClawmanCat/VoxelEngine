function(set_compiler_profile)
    add_compile_options(/EHsc)
    add_compile_options(/clang:-fbracket-depth=1024)
    add_compile_options(/clang:-Wall /clang:-Wextra)      # -Wpedantic will trigger ~4000 warnings in library code.
    add_compile_options(/clang:-Wno-deprecated-volatile)  # Mute warnings emitted by GLM.
    add_compile_options(/clang:-Wno-unknown-attributes)   # Mute warnings due to lack of C++20 attribute support.
    add_compile_options(/clang:-Wno-unused-private-field) # Unused members are used to disable move & copy on some objects.
    add_compile_options(/clang:-Wno-microsoft-cast)       # Cast function pointer to void pointer.
    add_compile_options(/clang:-Wno-unused-parameter)
endfunction()