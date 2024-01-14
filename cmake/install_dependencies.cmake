# Invokes Conan with the provided command and arguments (ARGN).
FUNCTION(RUN_CONAN COMMAND)
    EXECUTE_PROCESS(
        COMMAND conan ${COMMAND} ${ARGN}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
ENDFUNCTION()


# Install dependencies from the dependencies/conanfile.txt file. Dependencies are provided as targets in the CONAN_PKG namespace.
FUNCTION(INSTALL_CONAN_DEPENDENCIES)
    # Get OS name and build type to select the correct profile file.
    STRING(TOLOWER ${CMAKE_BUILD_TYPE}  BUILD_TYPE_LOWER)
    STRING(TOLOWER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_LOWER)

    # RelWithDebInfo -> FastDebug
    IF (${BUILD_TYPE_LOWER} STREQUAL "relwithdebinfo")
        SET(BUILD_TYPE_LOWER "fastdebug")
    ENDIF()


    SET(PROFILE_DIR   "${CMAKE_SOURCE_DIR}/dependencies/profiles")
    SET(CONAN_PROFILE "${PROFILE_DIR}/${SYSTEM_NAME_LOWER}_${BUILD_TYPE_LOWER}.conanprofile")


    # Skip if no conanfile exists.
    IF (NOT EXISTS "${CMAKE_SOURCE_DIR}/dependencies/conanfile.txt" AND NOT EXISTS "${CMAKE_SOURCE_DIR}/dependencies/conanfile.py")
        RETURN()
    ENDIF()

    # Use default profile if no specific profile is provided.
    IF (NOT EXISTS "${CONAN_PROFILE}")
        SET(CONAN_PROFILE "${PROFILE_DIR}/default.conanprofile")
    ENDIF()


    # Required if the compiler used for Conan differs from the one used for the project itself. (E.g. when mixing MSVC and Clang on Windows.)
    SET(CONAN_DISABLE_CHECK_COMPILER ON)


    # Run Conan and generate CMake targets for the installed dependencies.
    RUN_CONAN(install -b missing -g cmake -if ./out/conan/ -pr "${CONAN_PROFILE}" ./dependencies/)

    INCLUDE(${CMAKE_SOURCE_DIR}/out/conan/conanbuildinfo.cmake)
    CONAN_BASIC_SETUP(TARGETS)
ENDFUNCTION()


# Install dependencies from the dependencies/gitrepos.txt file. Dependencies are provided as targets in the GIT_PKG namespace.
# This is only used for libraries which don't have a published Conan package.
FUNCTION(INSTALL_GIT_DEPENDENCIES)
    INCLUDE(FetchContent)
    FIND_PACKAGE(Git REQUIRED)


    SET(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/out/git_pkgs/")
    SET(FETCHCONTENT_UPDATES_DISCONNECTED ON)
    SET(GIT_PKG_FILE "${CMAKE_SOURCE_DIR}/dependencies/gitrepos.txt")


    # Skip if no gitrepos file exists.
    IF (NOT EXISTS "${GIT_PKG_FILE}")
        RETURN()
    ENDIF()


    # Read and parse gitrepos file, skip if empty.
    FILE(READ ${GIT_PKG_FILE} GIT_REPO_LIST)

    IF (GIT_REPO_LIST STREQUAL "")
        RETURN()
    ENDIF()

    STRING(REGEX REPLACE ";" "\\\\;" GIT_REPO_LIST ${GIT_REPO_LIST}) # Escape existing semicolons.
    STRING(REGEX REPLACE "\n" ";" GIT_REPO_LIST ${GIT_REPO_LIST})    # Convert newlines to semicolons to line-separate contents into a list.


    FOREACH (LINE IN ITEMS ${GIT_REPO_LIST})
        STRING(REGEX REPLACE " " ";" LINE ${LINE}) # Convert spaces to semicolons to space-separate the line contents into a list.

        LIST(GET LINE 0 PKG_NAME)
        LIST(GET LINE 1 PKG_REPO)
        LIST(GET LINE 2 PKG_TAG)
        LIST(GET LINE 3 TGT_NAME)


        # Download and install project.
        FETCHCONTENT_DECLARE(
            "GIT_PKG_${PKG_NAME}"
            GIT_REPOSITORY "${PKG_REPO}"
            GIT_TAG "${PKG_TAG}"
            TIMEOUT 10
            TMP_DIR     "${CMAKE_SOURCE_DIR}/out/git_pkgs/${PKG_NAME}/tmp"
            LOG_DIR     "${CMAKE_SOURCE_DIR}/out/git_pkgs/${PKG_NAME}/log"
            STAMP_DIR   "${CMAKE_SOURCE_DIR}/out/git_pkgs/${PKG_NAME}/stamp"
            SOURCE_DIR  "${CMAKE_SOURCE_DIR}/out/git_pkgs/${PKG_NAME}/src"
            BINARY_DIR  "${CMAKE_SOURCE_DIR}/out/git_pkgs/${PKG_NAME}/bin"
            INSTALL_DIR "${CMAKE_SOURCE_DIR}/out/git_pkgs/${PKG_NAME}/bin"
        )

        FETCHCONTENT_MAKEAVAILABLE("GIT_PKG_${PKG_NAME}")


        # Resolve aliases
        GET_TARGET_PROPERTY(TGT_UNALIASED_NAME ${TGT_NAME} ALIASED_TARGET)

        IF (TGT_UNALIASED_NAME)
            SET(TGT_NAME ${TGT_UNALIASED_NAME})
        ENDIF()


        # Create wrapper library so we can namespace the target.
        ADD_LIBRARY("GIT_PKG::${PKG_NAME}" ALIAS "${TGT_NAME}")


        # Mark include directories as system includes so we don't get warnings from them.
        # TODO: It would be better to override TARGET_INCLUDE_DIRECTORIES, but the syntax is different for different types of target, so this is somewhat convoluted.
        GET_TARGET_PROPERTY(PKG_INCLUDES ${TGT_NAME} INCLUDE_DIRECTORIES)
        INCLUDE_DIRECTORIES(SYSTEM ${PKG_INCLUDES})


        # Print debug message that package was installed.
        MESSAGE(STATUS "Installed package ${PKG_NAME} from Git repository ${PKG_REPO} (Tag ${PKG_TAG}).")
    ENDFOREACH()
ENDFUNCTION()