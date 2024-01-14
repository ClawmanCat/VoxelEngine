FUNCTION(SET_CLANG_OPTION OPTION)
    IF (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
        ADD_COMPILE_OPTIONS("/clang:${OPTION}")
    ELSE()
        ADD_COMPILE_OPTIONS(${OPTION})
    ENDIF()
ENDFUNCTION()


FUNCTION(SET_COMPILER_WARNINGS)
    IF (VE_WARNINGS_ARE_ERRORS)
        SET_CLANG_OPTION(-Werror)
    ENDIF()

    SET_CLANG_OPTION(-Wall)
    SET_CLANG_OPTION(-Wextra)
    SET_CLANG_OPTION(-Wpedantic)

    SET_CLANG_OPTION(-Wnon-virtual-dtor)
    SET_CLANG_OPTION(-Wcast-align)
    SET_CLANG_OPTION(-Woverloaded-virtual)
    SET_CLANG_OPTION(-Wformat=2)


    # While it is technically UB to cast between void* and function pointer,
    # most operating systems require it to dynamically load symbols through GetProcAddress / dlsym.
    SET_CLANG_OPTION(-Wno-microsoft-cast)

    # This warning is obsolete, as the standard does not forbid this anymore in C++20.
    # (http://eel.is/c++draft/cpp.replace.general#15)
    SET_CLANG_OPTION(-Wno-gnu-zero-variadic-macro-arguments)

    # Macro-expansion may result in superfluous semicolons.
    # In some cases, the extra code required to prevent this would make things more unreadable.
    SET_CLANG_OPTION(-Wno-extra-semi)

    # Disable warnings for unused code.
    SET_CLANG_OPTION(-Wno-unused-variable)
    SET_CLANG_OPTION(-Wno-unused-parameter)
    SET_CLANG_OPTION(-Wno-unused-private-field)
    SET_CLANG_OPTION(-Wno-unused-function)
    SET_CLANG_OPTION(-Wno-unused-local-typedef)

    # Usage of magic_enum requires a large recursion depth.
    SET_CLANG_OPTION(-fbracket-depth=1024)

    # Don't hide part of the backtrace for template errors.
    SET_CLANG_OPTION(-ftemplate-backtrace-limit=0)

    # Allow lookup into dependent base classes.
    SET_CLANG_OPTION(-Wno-microsoft-template)

    # Conan packages don't have PDBs. We don't need a warning for every package that there is no debug info.
    ADD_LINK_OPTIONS(/ignore:4099)
ENDFUNCTION()