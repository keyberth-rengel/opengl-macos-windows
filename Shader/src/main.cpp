#if defined(_WIN32)
#include <GL/glew.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

#include "Shader.h"

// Triangulo base que se reutiliza 4 veces.
const float vertices[] = {
    -0.15f, -0.15f, 0.0f, 1.0f, 0.0f, 0.0f,
     0.15f, -0.15f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.0f,   0.15f, 0.0f, 0.0f, 0.0f, 1.0f,
};

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

int main()
{
    // Inicializa GLFW y configura OpenGL 3.3 core.
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1200, 800, "Mi primera ventana", NULL, NULL);
    if (!window)
    {
        std::cout << "Error en la inicializacion de la ventana\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Carga las funciones de OpenGL segun la plataforma.
#if defined(_WIN32)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error en la inicializacion de GLEW\n";
        glfwTerminate();
        return -1;
    }
#else
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cout << "Error en la inicializacion de GLAD\n";
        glfwTerminate();
        return -1;
    }
#endif

    int fbWidth;
    int fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // Carga los shaders desde archivos externos.
    Shader myShader("res/Shader/vertexShader.glsl", "res/Shader/fragmentShader.glsl");

    // Un solo VAO/VBO para reutilizar el mismo triangulo.
    unsigned int VBO;
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Tres triangulos orbitan y el cuarto se mueve horizontalmente.
    const float PI = 3.14159265f;
    const float radius = 0.85f;
    const float orbitPhases[] = {0.0f, 2.0f * PI / 3.0f, 4.0f * PI / 3.0f};
    const float extraRange = 0.85f;
    const float extraY = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        const float time = static_cast<float>(glfwGetTime());
        const float xValue = std::sin(time) / 2.0f + 0.5f;

        glClearColor(0.1f, 0.2f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        myShader.use();
        glBindVertexArray(VAO);
        myShader.setFloat("xColor", xValue);

        // Dibuja los 3 triangulos que forman el circulo.
        for (int i = 0; i < 3; ++i)
        {
            const float phase = time + orbitPhases[i];
            const float xOffset = radius * std::cos(phase);
            const float yOffset = radius * std::sin(phase);

            myShader.setFloat("xOffset", xOffset);
            myShader.setFloat("yOffset", yOffset);
            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        // Dibuja el triangulo adicional con movimiento horizontal.
        myShader.setFloat("xOffset", std::sin(time) * extraRange);
        myShader.setFloat("yOffset", extraY);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
