macro(run_conan)
    include(cmake_conan)

    # Read remote list.
    file(READ "${CMAKE_SOURCE_DIR}/dependencies/remotes.txt" remotes)
    string(REPLACE "\n" "${Esc};" remotes ${remotes})

    # Parse name / URL pairs and add remotes.
    foreach(remote IN ITEMS ${remotes})
        separate_arguments(remote)

        list(GET remote 0 name)
        list(GET remote 1 url)

        conan_add_remote(NAME ${name} URL ${url})
    endforeach()

    # Use a profile if we're in Windows, since we need to set the compiler and pick between MD and MDd.
    string(TOLOWER ${CMAKE_BUILD_TYPE} config)
    if(WIN32)
        set(profile windows_${config}.conanprofile)
    endif()

    # conan_cmake_run from cmake_conan.cmake does not seem to work,
    # so just use it to add the remotes and call Conan manually.
    execute_process(
            COMMAND conan install
            -b missing
            -g cmake
            -if ./out/conan/
            -pr ./dependencies/${profile}
            ./dependencies/
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )

    include("${CMAKE_SOURCE_DIR}/out/conan/conanbuildinfo.cmake")

    # On Windows, we have to trick Conan into thinking we're using MSVC so it doesn't use GCC-style arguments.
    if (WIN32)
        set(CONAN_DISABLE_CHECK_COMPILER ON)
    endif()

    conan_basic_setup(TARGETS)
endmacro()