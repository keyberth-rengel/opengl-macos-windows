cmake_minimum_required(VERSION 3.15)
project(LSystemForest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${PROJECT_SOURCE_DIR}/src)

find_package(glfw3 REQUIRED)

if (WIN32)
    find_package(GLEW REQUIRED)

    add_executable(LSystemForest
        src/main.cpp
        src/Shader.cpp
    )

    target_link_libraries(LSystemForest PRIVATE glfw GLEW::GLEW opengl32)
else()
    add_executable(LSystemForest
        src/main.cpp
        src/Shader.cpp
    )

    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)

    target_link_libraries(LSystemForest PRIVATE
        glfw
        ${OPENGL_LIBRARY}
        ${COCOA_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
    )
endif()
