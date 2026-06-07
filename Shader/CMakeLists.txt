cmake_minimum_required(VERSION 3.10)

project(Shader)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.3 REQUIRED CONFIG)

add_executable(Shader
    src/main.cpp
    src/Shader.cpp
)

target_include_directories(Shader PRIVATE
    include
    src
)

if (WIN32)
    find_package(GLEW REQUIRED)
    target_link_libraries(Shader PRIVATE
        glfw
        GLEW::GLEW
        opengl32
    )
else()
    target_sources(Shader PRIVATE libs/gl.c)
    target_link_libraries(Shader PRIVATE
        glfw
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()
