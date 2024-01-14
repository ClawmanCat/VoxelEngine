FUNCTION(CONFIGURE_COMPILER)
    # Enable compiler-specific compatibility flags and warnings.
    IF (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        INCLUDE(profiles/compatibility_msvc)
        INCLUDE(profiles/warnings_msvc)

        SET_COMPATIBILITY_FLAGS()
        SET_COMPILER_WARNINGS()
    ELSEIF (CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")
        INCLUDE(profiles/compatibility_clang)
        INCLUDE(profiles/warnings_clang)

        SET_COMPATIBILITY_FLAGS()
        SET_COMPILER_WARNINGS()
    ELSEIF (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        INCLUDE(profiles/warnings_gcc)

        SET_COMPILER_WARNINGS()
    ENDIF()


    # Set required preprocessor definitions.
    STRING(TOUPPER ${VE_GRAPHICS_API} GFXAPI_UPPER)

    ADD_COMPILE_DEFINITIONS(SDL_MAIN_HANDLED)
    ADD_COMPILE_DEFINITIONS("VE_GRAPHICS_API=${VE_GRAPHICS_API}")
    ADD_COMPILE_DEFINITIONS("VE_GRAPHICS_API_${GFXAPI_UPPER}=1")


    IF (WIN32)
        ADD_COMPILE_DEFINITIONS(_CRT_SECURE_NO_WARNINGS)

        # Several libraries unfortunately include Windows.h through their headers.
        # Try and disable as many of the Windows.h macros as possible to prevent them from breaking our code.
        ADD_COMPILE_DEFINITIONS(NOGDI)
        ADD_COMPILE_DEFINITIONS(WIN32_LEAN_AND_MEAN)
        ADD_COMPILE_DEFINITIONS(NOMINMAX)
    ENDIF()
ENDFUNCTION()