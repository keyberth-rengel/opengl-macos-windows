#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>

const unsigned int WINDOW_WIDTH = 800;
const unsigned int WINDOW_HEIGHT = 600;

enum class ShapeType
{
    Triangle,
    Rectangle,
    Trapezoid,
    Hexagon,
    Cross,
    Castle,
    Diamond,
    Rocket
};

const ShapeType ACTIVE_SHAPE = ShapeType::Diamond;

const float TRIANGLE_VERTICES[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f, 0.5f, 0.0f};

const float RECTANGLE_VERTICES[] = {
    -0.5f, -0.4f, 0.0f,
    0.5f, -0.4f, 0.0f,
    0.5f, 0.4f, 0.0f,
    0.5f, 0.4f, 0.0f,
    -0.5f, 0.4f, 0.0f,
    -0.5f, -0.4f, 0.0f};

const float TRAPEZOID_VERTICES[] = {
    -0.6f, -0.5f, 0.0f,
    0.6f, -0.5f, 0.0f,
    0.3f, 0.5f, 0.0f,
    0.3f, 0.5f, 0.0f,
    -0.3f, 0.5f, 0.0f,
    -0.6f, -0.5f, 0.0f};

const float H = 0.433f;
const float HEXAGON_VERTICES[] = {
    0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.25f, H, 0.0f,
    0.0f, 0.0f, 0.0f, 0.25f, H, 0.0f, -0.25f, H, 0.0f,
    0.0f, 0.0f, 0.0f, -0.25f, H, 0.0f, -0.5f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, -0.5f, 0.0f, 0.0f, -0.25f, -H, 0.0f,
    0.0f, 0.0f, 0.0f, -0.25f, -H, 0.0f, 0.25f, -H, 0.0f,
    0.0f, 0.0f, 0.0f, 0.25f, -H, 0.0f, 0.5f, 0.0f, 0.0f};

const float CROSS_VERTICES[] = {
    // Barra vertical
    -0.20f, -0.60f, 0.0f, 0.20f, -0.60f, 0.0f, 0.20f, 0.60f, 0.0f,
    0.20f, 0.60f, 0.0f, -0.20f, 0.60f, 0.0f, -0.20f, -0.60f, 0.0f,
    // Barra horizontal
    -0.60f, -0.20f, 0.0f, 0.60f, -0.20f, 0.0f, 0.60f, 0.20f, 0.0f,
    0.60f, 0.20f, 0.0f, -0.60f, 0.20f, 0.0f, -0.60f, -0.20f, 0.0f};

const float CASTLE_VERTICES[] = {
    // Torre izquierda (rectángulo)
    -0.75f, -0.65f, 0.0f, -0.35f, -0.65f, 0.0f, -0.35f, 0.35f, 0.0f,
    -0.35f, 0.35f, 0.0f, -0.75f, 0.35f, 0.0f, -0.75f, -0.65f, 0.0f,
    // Torre derecha (rectángulo)
    0.35f, -0.65f, 0.0f, 0.75f, -0.65f, 0.0f, 0.75f, 0.35f, 0.0f,
    0.75f, 0.35f, 0.0f, 0.35f, 0.35f, 0.0f, 0.35f, -0.65f, 0.0f,
    // Cuerpo central (rectángulo)
    -0.35f, -0.65f, 0.0f, 0.35f, -0.65f, 0.0f, 0.35f, 0.05f, 0.0f,
    0.35f, 0.05f, 0.0f, -0.35f, 0.05f, 0.0f, -0.35f, -0.65f, 0.0f,
    // Techo central (triángulo)
    -0.35f, 0.05f, 0.0f, 0.00f, 0.55f, 0.0f, 0.35f, 0.05f, 0.0f,
    // Techo torre izquierda (triángulo)
    -0.75f, 0.35f, 0.0f, -0.55f, 0.65f, 0.0f, -0.35f, 0.35f, 0.0f,
    // Techo torre derecha (triángulo)
    0.35f, 0.35f, 0.0f, 0.55f, 0.65f, 0.0f, 0.75f, 0.35f, 0.0f};

const float DIAMOND_VERTICES[] = {
    // Diamante tipo gema: parte superior plana + punta inferior
    -0.30f, 0.45f, 0.0f, 0.30f, 0.45f, 0.0f, 0.48f, 0.18f, 0.0f,
    -0.30f, 0.45f, 0.0f, 0.48f, 0.18f, 0.0f, 0.00f, -0.58f, 0.0f,
    -0.30f, 0.45f, 0.0f, 0.00f, -0.58f, 0.0f, -0.48f, 0.18f, 0.0f};

const float ROCKET_VERTICES[] = {
    // Punta superior (triángulo)
    -0.15f, 0.45f, 0.0f, 0.00f, 0.82f, 0.0f, 0.15f, 0.45f, 0.0f,
    // Cuerpo superior (rectángulo)
    -0.15f, 0.10f, 0.0f, 0.15f, 0.10f, 0.0f, 0.15f, 0.45f, 0.0f,
    0.15f, 0.45f, 0.0f, -0.15f, 0.45f, 0.0f, -0.15f, 0.10f, 0.0f,
    // Ala/media sección (trapecio)
    -0.15f, 0.10f, 0.0f, 0.15f, 0.10f, 0.0f, 0.55f, -0.15f, 0.0f,
    0.55f, -0.15f, 0.0f, -0.55f, -0.15f, 0.0f, -0.15f, 0.10f, 0.0f,
    // Cola inferior (rectángulo)
    -0.12f, -0.52f, 0.0f, 0.12f, -0.52f, 0.0f, 0.12f, -0.15f, 0.0f,
    0.12f, -0.15f, 0.0f, -0.12f, -0.15f, 0.0f, -0.12f, -0.52f, 0.0f};

struct ShapeData
{
    const float *vertices;
    GLsizei vertexCount;
    size_t sizeInBytes;
    GLenum drawMode;
};

const char *VERTEX_SHADER_SOURCE = R"(
#version 410 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

const char *FRAGMENT_SHADER_SOURCE = R"(
#version 410 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(0.95, 0.65, 0.20, 1.0);
}
)";

void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
bool buildShaderProgram(unsigned int &shaderProgram);
ShapeData getShapeData(ShapeType shapeType);

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Error al inicializar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow *window = glfwCreateWindow(
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        "OpenGL en Mac",
        nullptr,
        nullptr);

    if (window == nullptr)
    {
        std::cerr << "Error al crear la ventana GLFW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cerr << "Error al inicializar GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    unsigned int shaderProgram;
    if (!buildShaderProgram(shaderProgram))
    {
        glfwTerminate();
        return -1;
    }

    unsigned int VBO;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    const ShapeData shapeData = getShapeData(ACTIVE_SHAPE);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, shapeData.sizeInBytes, shapeData.vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    int fbWidth;
    int fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.10f, 0.12f, 0.18f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(shapeData.drawMode, 0, shapeData.vertexCount);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

bool buildShaderProgram(unsigned int &shaderProgram)
{
    int success;
    char infoLog[512];

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VERTEX_SHADER_SOURCE, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Error compilando vertex shader: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        return false;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &FRAGMENT_SHADER_SOURCE, nullptr);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Error compilando fragment shader: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Error enlazando shader program: " << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return true;
}

ShapeData getShapeData(ShapeType shapeType)
{
    switch (shapeType)
    {
    case ShapeType::Triangle:
        return {TRIANGLE_VERTICES, 3, sizeof(TRIANGLE_VERTICES), GL_TRIANGLES};
    case ShapeType::Rectangle:
        return {RECTANGLE_VERTICES, 6, sizeof(RECTANGLE_VERTICES), GL_TRIANGLES};
    case ShapeType::Trapezoid:
        return {TRAPEZOID_VERTICES, 6, sizeof(TRAPEZOID_VERTICES), GL_TRIANGLES};
    case ShapeType::Hexagon:
        return {HEXAGON_VERTICES, 18, sizeof(HEXAGON_VERTICES), GL_TRIANGLES};
    case ShapeType::Cross:
        return {CROSS_VERTICES, 12, sizeof(CROSS_VERTICES), GL_TRIANGLES};
    case ShapeType::Castle:
        return {CASTLE_VERTICES, 27, sizeof(CASTLE_VERTICES), GL_TRIANGLES};
    case ShapeType::Diamond:
        return {DIAMOND_VERTICES, 9, sizeof(DIAMOND_VERTICES), GL_TRIANGLES};
    case ShapeType::Rocket:
        return {ROCKET_VERTICES, 21, sizeof(ROCKET_VERTICES), GL_TRIANGLES};
    default:
        return {TRIANGLE_VERTICES, 3, sizeof(TRIANGLE_VERTICES), GL_TRIANGLES};
    }
}
