cmake_minimum_required(VERSION 3.14)

project(demo)

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        eventbus::eventbus
)
