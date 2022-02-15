# Runs a Conan command with the given name and variadic arguments.
function(conan_command name)
    execute_process(
        COMMAND conan ${name}
        ${ARGN}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
endfunction()


function(select_conan_profile return_var)
    include(utility)


    get_platform_name(os)
    string(TOLOWER ${CMAKE_BUILD_TYPE} configuration)


    set(target_profile "${CMAKE_SOURCE_DIR}/dependencies/profiles/${os}_${configuration}.conanprofile")

    if (EXISTS ${target_profile})
        set(${return_var} ${target_profile} PARENT_SCOPE)
    else()
        set(${return_var} "default.conanprofile" PARENT_SCOPE)
    endif()
endfunction()


# Runs Conan to set up the dependencies for the project, as specified in the conanfile.txt,
# using the given Conan profile and using the remote list in conanremotes.txt.
function(run_conan profile)
    include(utility)


    # Add remotes from file.
    function(conan_add_remote name url)
        conan_command(remote add ${name} ${url} --force)
    endfunction()

    parse_kv_file("${CMAKE_SOURCE_DIR}/dependencies/conanremotes.txt" conan_add_remote)


    # Install dependencies.
    conan_command(
        install
        -b missing
        -g cmake
        -if ./out/conan/
        -pr ${profile}
        ./dependencies/
    )

    include("${CMAKE_SOURCE_DIR}/out/conan/conanbuildinfo.cmake")


    # Conan will assume it needs to use GCC-style command line arguments when using Clang on Windows,
    # even when Clang-CL is used, which requires MSVC-style arguments instead.
    # Just disabling the compiler check fixes this issue.
    if (WIN32)
        set(CONAN_DISABLE_CHECK_COMPILER ON)
    endif()


    # Make installed dependencies available in CMake.
    conan_basic_setup(TARGETS)
endfunction()