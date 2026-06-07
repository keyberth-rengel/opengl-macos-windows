cmake_minimum_required(VERSION 3.10)

project(OpenGLProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.3 REQUIRED CONFIG)

include_directories(include)

add_executable(OpenGLProject
    src/main.cpp
    libs/gl.c
)

target_link_libraries(OpenGLProject
    glfw
    "-framework OpenGL"
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
)
