#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#if defined(_WIN32)
#include <GL/glew.h>
#else
#include <glad/gl.h>
#endif

#include <cmath>
#include <iostream>

const unsigned int WINDOW_WIDTH = 900;
const unsigned int WINDOW_HEIGHT = 600;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
bool buildShaderProgram(unsigned int& shaderProgram);

const char* VERTEX_SHADER_SOURCE = R"(
#version 410 core
// Atributo de posicion (location 0) y color (location 1).
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vColor;

void main()
{
    gl_Position = vec4(aPos, 1.0);
    vColor = aColor;
}
)";

const char* FRAGMENT_SHADER_SOURCE = R"(
#version 410 core
in vec3 vColor;
out vec4 FragColor;

// Uniform de tiempo para animar intensidad de color.
uniform float u_time;

void main()
{
    // Variacion suave en el tiempo.
    float pulse = 0.65 + 0.35 * sin(u_time * 2.0);
    FragColor = vec4(vColor * pulse, 1.0);
}
)";

int main()
{
    // 1) Inicializacion de GLFW y contexto OpenGL.
    // Si esta parte falla, no existe ventana ni contexto para dibujar.
    if (!glfwInit())
    {
        std::cerr << "Error al inicializar GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "PC2 - EBO + Atributos + Uniform", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Error al crear la ventana GLFW\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // 2) Carga de funciones OpenGL segun plataforma:
    // - Windows: GLEW
    // - macOS/Linux: GLAD
#if defined(_WIN32)
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Error al inicializar GLEW\n";
        glfwTerminate();
        return -1;
    }
#else
    if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress))
    {
        std::cerr << "Error al inicializar GLAD\n";
        glfwTerminate();
        return -1;
    }
#endif

    // Datos de geometria:
    // 4 vertices unicos (sin duplicar) en estructura intercalada:
    // [x, y, z, r, g, b]
    float vertices[] = {
         0.45f,  0.30f, 0.0f,   1.00f, 0.75f, 0.25f, // top-right
         0.45f, -0.30f, 0.0f,   1.00f, 0.45f, 0.10f, // bottom-right
        -0.45f, -0.30f, 0.0f,   0.95f, 0.30f, 0.05f, // bottom-left
        -0.45f,  0.30f, 0.0f,   1.00f, 0.60f, 0.15f  // top-left
    };

    // Indices para reutilizar vertices con EBO.
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // 4) Objetos de GPU:
    // VAO guarda configuracion de atributos/buffers.
    // VBO guarda vertices.
    // EBO guarda indices.
    unsigned int VBO;
    unsigned int VAO;
    unsigned int EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // 5) Definicion de como leer cada vertice en memoria:
    // stride = 6 floats por vertice (3 posicion + 3 color).
    // location 0 -> posicion (3 floats), stride total = 6 floats.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // location 1 -> color (offset = 3 floats).
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    unsigned int shaderProgram;
    if (!buildShaderProgram(shaderProgram))
    {
        glfwTerminate();
        return -1;
    }

    // 6) Uniforms:
    // buscamos la ubicacion de u_time una sola vez para no repetir costo cada frame.
    // Obtener location una sola vez.
    int uTimeLocation = glGetUniformLocation(shaderProgram, "u_time");

    int fbWidth;
    int fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // 7) Loop de render:
    // limpiar pantalla, actualizar uniform dinamico, dibujar, presentar frame.
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.08f, 0.10f, 0.16f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        float currentTime = static_cast<float>(glfwGetTime());
        glUniform1f(uTimeLocation, currentTime);

        glBindVertexArray(VAO);
        // Dibujo indexado con EBO.
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 8) Liberacion de recursos de GPU y cierre limpio.
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    // Permite cerrar rapido la app con ESC.
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    (void)window;
    // Ajusta el area de dibujo real cuando cambia el tamano de la ventana.
    glViewport(0, 0, width, height);
}

bool buildShaderProgram(unsigned int& shaderProgram)
{
    // Compila Vertex + Fragment Shader y luego los enlaza en un solo programa.
    // Devuelve false si alguna etapa falla, mostrando el error en consola.
    int success;
    char infoLog[512];

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &VERTEX_SHADER_SOURCE, nullptr);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Error compilando vertex shader: " << infoLog << "\n";
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
        std::cerr << "Error compilando fragment shader: " << infoLog << "\n";
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
        std::cerr << "Error enlazando shader program: " << infoLog << "\n";
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return true;
}
