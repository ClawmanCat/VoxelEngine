# Loads and applies a settings preset for the currently used compiler,
# enabling recommended warnings and errors.
function(set_compiler_profile)
    include(utility)
    get_compiler_name(profile)


    if (EXISTS ${CMAKE_SOURCE_DIR}/cmake/profiles/${profile}.cmake)
        message(STATUS "Using ${profile} compiler settings profile.")

        include("profiles/${profile}")
        load_compiler_profile()
    else()
        message(WARNING "Unknown compiler: ${profile}. No settings profile will be applied.")
    endif()
endfunction()