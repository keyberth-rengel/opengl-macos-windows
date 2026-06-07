#pragma once

#if defined(_WIN32)
#include <GL/glew.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader
{
private:
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vertexShaderFile;
    std::ifstream fragmentShaderFile;
    std::stringstream vertexShaderStream;
    std::stringstream fragmentShaderStream;
    const char* vertexShaderData;
    const char* fragmentShaderData;
    unsigned int vertex;
    unsigned int fragment;

public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    void setBool(const std::string& name, bool value);
    void setInt(const std::string& name, int value);
    void setFloat(const std::string& name, float value);
};
