cmake_minimum_required(VERSION 3.15)
project(SolarSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glfw3 REQUIRED)

add_executable(SolarSystem
    src/main.cpp
        src/Shader.cpp
            src/vendor/stb_image/stb_image.cpp
            )

            target_include_directories(SolarSystem PRIVATE
                include
                    src
                        src/vendor
                        )

                        if (WIN32)
                            find_package(GLEW REQUIRED)
                                target_link_libraries(SolarSystem PRIVATE glfw GLEW::GLEW opengl32)
                                else()
                                    target_sources(SolarSystem PRIVATE libs/gl.c)
                                        target_link_libraries(SolarSystem PRIVATE
                                                glfw
                                                        "-framework OpenGL"
                                                                "-framework Cocoa"
                                                                        "-framework IOKit"
                                                                                "-framework CoreVideo"
                                                                                    )
                                                                                    endif()
                                                                                    