cmake_minimum_required(VERSION 3.14)

project(eventbus)

set(project_headers
    include/eventbus/detail/function_traits.hpp
    include/eventbus/event_bus.hpp
)

add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})
add_library(dp::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} INTERFACE include)

target_link_libraries(${PROJECT_NAME} INTERFACE project_options)

if(MSVC_IDE)
    add_custom_target(${PROJECT_NAME}.headers SOURCES ${project_headers})
endif()

if(EVENTBUS_BUILD_TESTS)
    set(project_test_sources
        test/event_bus_tests.cpp
    )
    set(project_test_name ${PROJECT_NAME}.tests)
    add_executable(${project_test_name} ${project_test_sources})
    target_link_libraries(${project_test_name}
        PUBLIC
            gtest
            gtest_main
            gmock
            ${PROJECT_NAME}
    )

    include(GoogleTest)
    gtest_add_tests(
        TARGET ${project_test_name}
        SOURCES ${project_test_sources}
    )
endif()
