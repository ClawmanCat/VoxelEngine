FUNCTION(SET_COMPILER_WARNINGS)
    IF (VE_WARNINGS_ARE_ERRORS)
        ADD_COMPILE_OPTIONS(-Werror)
    ENDIF()

    ADD_COMPILE_OPTIONS(-Wall)
    ADD_COMPILE_OPTIONS(-Wextra)
    ADD_COMPILE_OPTIONS(-Wpedantic)

    ADD_COMPILE_OPTIONS(-Wnon-virtual-dtor)
    ADD_COMPILE_OPTIONS(-Wcast-align)
    ADD_COMPILE_OPTIONS(-Woverloaded-virtual)
    ADD_COMPILE_OPTIONS(-Wformat=2)


    # While it is technically UB to cast between void* and function pointer,
    # most operating systems require it to dynamically load symbols through GetProcAddress / dlsym.
    ADD_COMPILE_OPTIONS(-Wno-microsoft-cast)

    # This warning is obsolete, as the standard does not forbid this anymore in C++20.
    # (http://eel.is/c++draft/cpp.replace.general#15)
    ADD_COMPILE_OPTIONS(-Wno-gnu-zero-variadic-macro-arguments)

    # Macro-expansion may result in superfluous semicolons.
    # In some cases, the extra code required to prevent this would make things more unreadable.
    ADD_COMPILE_OPTIONS(-Wno-extra-semi)

    # Disable warnings for unused code.
    ADD_COMPILE_OPTIONS(-Wno-unused-variable)
    ADD_COMPILE_OPTIONS(-Wno-unused-parameter)
    ADD_COMPILE_OPTIONS(-Wno-unused-private-field)
    ADD_COMPILE_OPTIONS(-Wno-unused-function)
    ADD_COMPILE_OPTIONS(-Wno-unused-local-typedef)

    # Usage of magic_enum requires a large recursion depth.
    ADD_COMPILE_OPTIONS(-fbracket-depth=1024)

    # Don't hide part of the backtrace for template errors.
    ADD_COMPILE_OPTIONS(-ftemplate-backtrace-limit=0)

    # Allow lookup into dependent base classes.
    ADD_COMPILE_OPTIONS(-Wno-microsoft-template)

    # Conan packages don't have PDBs. We don't need a warning for every package that there is no debug info.
    ADD_LINK_OPTIONS(/ignore:4099)
ENDFUNCTION()