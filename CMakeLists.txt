cmake_minimum_required(VERSION 3.14)

project(eventbus)

set(CXX_STANDARD 17)
set(CXX_STANDARD_REQUIRED ON)

option(EVENTBUS_BUILD_TESTS "Turn on to build unit tests." OFF)

if(EVENTBUS_BUILD_TESTS)
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    set(CPM_VERSION 0.12)

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
endif()

add_subdirectory(eventbus)
add_subdirectory(demo)
