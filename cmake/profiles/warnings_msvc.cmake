FUNCTION(SET_COMPILER_WARNINGS)
    IF (VE_WARNINGS_ARE_ERRORS)
        ADD_COMPILE_OPTIONS(/WX)
    ENDIF()


    # Enable extra warnings.
    ADD_COMPILE_OPTIONS(/W4)


    # Disable warnings in external libraries.
    ADD_COMPILE_OPTIONS(/external:W0)


    # Add additional warnings not covered by /W4.
    ADD_COMPILE_OPTIONS(
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
    ADD_COMPILE_OPTIONS(
        /wd4068 # Don't warn on pragmas intended for different compilers
        /wd4103 # Pragma push without pop. These get triggered from included files, even if they are marked as external,
                # since they change the alignment of the file that includes them too
        /wd4100 # Unreferenced parameters may exist if they're only used in either debug or release mode
        /wd4127 # If statement can be constexpr.
                # Triggers incorrectly on combined if-statements where only part of the statement is constexpr
        /wd4458 # Declaration hides class member.
                # Shadowing is done intentionally in many places, e.g.: void set_x(T x) { this->x = x; }
    )
ENDFUNCTION()