cmake_minimum_required(VERSION 3.14)

project(eventbus)

set(project_headers
    include/eventbus/utils/singleton.hpp
    include/eventbus/utils/callable.hpp
    include/eventbus/event_bus.hpp
)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} INTERFACE include)

# required C++17
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_17)

if(MSVC_IDE)
    add_custom_target(${PROJECT_NAME}.headers SOURCES ${project_headers})
endif()

if(EVENTBUS_BUILD_TESTS)
    set(project_test_sources
        test/main.cpp
        test/callable_tests.cpp
    )

    add_executable(${PROJECT_NAME}.tests ${project_test_sources})
    target_link_libraries(${PROJECT_NAME}.tests
        PUBLIC
            gtest
            gmock
            ${PROJECT_NAME}
    )
endif()
