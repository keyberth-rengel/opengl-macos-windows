cmake_minimum_required(VERSION 3.10)

project(NaveEspacial)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 3.3 REQUIRED CONFIG)

if(WIN32)
    add_definitions(-DGLEW_STATIC)
endif()

add_executable(NaveEspacial
    src/main.cpp
    src/Shader.cpp
)

target_include_directories(NaveEspacial PRIVATE
    src
    ../Shader/include
)

if(WIN32)
    find_package(GLEW REQUIRED)
    target_include_directories(NaveEspacial PRIVATE ${GLEW_INCLUDE_DIRS})
    target_link_libraries(NaveEspacial PRIVATE glfw GLEW::GLEW opengl32)
else()
    target_sources(NaveEspacial PRIVATE ../Shader/libs/gl.c)
    target_link_libraries(NaveEspacial PRIVATE
        glfw
        "-framework OpenGL"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreVideo"
    )
endif()
