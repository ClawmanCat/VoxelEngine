function(set_debug_settings)
    add_compile_options(
        /MDd
        /O0 /Od
        /GS /guard:cf /RTC1
        /DEBUG:FULL /Zi /analyze /fsanitize /sdl /permissive-
    )
endfunction()


function(set_release_settings)
    add_compile_options(
        /MD
        /O2 /Ob3 /Oi /Ot
        /arch:AVX2
        /GF /GL /GS- /Gw
        /Qfast_transcendentals
        /QIntel-jcc-erratum
        /Qpar
    )
endfunction()


function(set_compiler_profile)
    # CMake seems to have issues selecting correct options for MSVC (e.g. using /02 on debug builds which prevents /RTC1),
    # so we will set all options manually.
    set(CMAKE_CXX_FLAGS "")

    add_compile_options(/std:c++latest)
    add_compile_options(/DVE_QUIET_GRAPHICS /DVE_QUIET_RES_OWNER)
    add_compile_options(/EHsc /GR) # TODO: Disable exceptions & RTTI.


    # Disable warnings from external libraries.
    add_compile_options(/experimental:external /external:W0)

    foreach (dir IN ITEMS ${CONAN_INCLUDE_DIRS})
        add_compile_options(/external:I${dir})
    endforeach()


    if(${CMAKE_BUILD_TYPE} MATCHES DEBUG)
        set_debug_settings()
    else()
        # set_release_settings()
    endif()


    # Enable Warnings
    add_compile_options(/W4)

    add_compile_options(
        /w34265 # No virtual dtor
        /w34464 # ../ in relative include
        /w34547 # No effect operation before comma
        /w34548 # No effect operation before comma
        /w34555 # No effect expression
        /w34596 # Illegal qualified name
        /w34643 # Forward declaration of identifier in namespace std
        /w35042 # Illegal inline specifier
        /w35041 # Deprecated out-of-line definition of constexpr static member
        /w34749 # Use of offsetof on non-POD type
        /w34946 # reinterpret_cast where static_cast is possible
        /w35022 # Multiple move constructors
        /w35023 # Multiple move assignment operators
        /w34242 # Conversion with possible loss of data
        /w34263 # Function marked override does not override any function
        /w34287 # Unsigned / negative constant mismatch
        /w34289 # Use of loop control variable outside loop
        /w34296 # Expression is always true / false
        /w34311 # Pointer truncation
        /w34545 # Expression before comma evaluates to function without argument list
        /w34619 # Unknown warning in pragma
        /w34826 # Conversion with sign extension
        /w34905 # Wide string to LPSTR
        /w34906 # String to LPWSTR
        /w34928 # Illegal copy-initialization
    )

    # Disable different warnings.
    add_compile_options(
        /wd4068 # Don't warn on pragmas intended for different compilers
        /wd4103 # Pragma push without pop. These get triggered from included files, even if they are marked as external,
                # since they change the alignment of the file that includes them too
        /wd4100 # Unreferenced parameters may exist if they're only used in either debug or release mode
        /wd4127 # If statement can be constexpr.
                # Triggers incorrectly on combined if-statements where only part of the statement is constexpr
        /wd4458 # Declaration hides class member.
                # Codebase contains many methods of the form void set_x(T x) { this->x = x; }
    )
endfunction()