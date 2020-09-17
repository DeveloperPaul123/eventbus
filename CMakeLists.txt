cmake_minimum_required(VERSION 3.14)

project(eventbus)

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

option(EVENTBUS_BUILD_TESTS "Build unit tests." ON)

include(CompilerWarnings)
add_library(project_warnings INTERFACE)
set_project_warnings(project_warnings)

add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

if(EVENTBUS_BUILD_TESTS)
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    set(CPM_VERSION 0.27.3)

    if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
        message(STATUS "Downloading CPM.cmake")
        file(DOWNLOAD https://raw.githubusercontent.com/TheLartians/CPM/v${CPM_VERSION}/cmake/CPM.cmake ${CPM_DOWNLOAD_LOCATION})
    endif(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})

    include(${CPM_DOWNLOAD_LOCATION})

    CPMAddPackage(
        NAME googletest
        GITHUB_REPOSITORY google/googletest
        GIT_TAG release-1.8.1
        VERSION 1.8.1
        OPTIONS
            "INSTALL_GTEST OFF"
            "gtest_force_shared_crt ON"
    )

    # this needs to be in the top level directory
    # before any calls to `add_subdirectory()` for
    # ctest to be set up properly
    enable_testing()
endif()

add_subdirectory(eventbus)
add_subdirectory(demo)
