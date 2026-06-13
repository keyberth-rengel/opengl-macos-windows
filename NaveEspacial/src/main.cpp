#if defined(_WIN32)
#include <GL/glew.h>
#else
#include <glad/gl.h>
#endif

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <iostream>

#include "Shader.h"

struct Asteroid
{
    float x;
    float y;
    float speed;
    float scale;
};

// Nave simple formada por dos triangulos.
const float shipVertices[] = {
    -0.10f, -0.08f, 0.0f, 0.4f, 0.6f, 1.0f,
     0.10f, -0.08f, 0.0f, 0.4f, 0.6f, 1.0f,
     0.00f,  0.20f, 0.0f, 0.8f, 0.9f, 1.0f,

    -0.05f, -0.08f, 0.0f, 0.2f, 0.3f, 0.8f,
     0.05f, -0.08f, 0.0f, 0.2f, 0.3f, 0.8f,
     0.00f, -0.20f, 0.0f, 0.2f, 0.3f, 0.8f,
};

// Asteroide simple con forma de octagono.
const float asteroidVertices[] = {
     0.00f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.00f,  0.12f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.085f,  0.085f, 0.0f, 0.6f, 0.6f, 0.6f,

     0.00f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.085f,  0.085f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.12f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,

     0.00f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.12f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.085f, -0.085f, 0.0f, 0.6f, 0.6f, 0.6f,

     0.00f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.085f, -0.085f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.00f, -0.12f, 0.0f, 0.6f, 0.6f, 0.6f,

     0.00f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.00f, -0.12f, 0.0f, 0.6f, 0.6f, 0.6f,
    -0.085f, -0.085f, 0.0f, 0.6f, 0.6f, 0.6f,

     0.00f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
    -0.085f, -0.085f, 0.0f, 0.6f, 0.6f, 0.6f,
    -0.12f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,

     0.00f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
    -0.12f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
    -0.085f,  0.085f, 0.0f, 0.6f, 0.6f, 0.6f,

     0.00f,  0.00f, 0.0f, 0.6f, 0.6f, 0.6f,
    -0.085f,  0.085f, 0.0f, 0.6f, 0.6f, 0.6f,
     0.00f,  0.12f, 0.0f, 0.6f, 0.6f, 0.6f,
};

const int asteroidVertexCount = 24;

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void setupBuffer(unsigned int& vao, unsigned int& vbo, const float* vertices, size_t vertexBytes)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexBytes, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

int main()
{
    const float shipHalfWidth = 0.10f;
    const float shipHalfHeight = 0.20f;
    const float normalShipSpeed = 0.9f;
    const float normalAsteroidSpeedFactor = 1.0f;
    float shipX = 0.0f;
    float shipY = -0.65f;
    float lastFrame = 0.0f;

    Asteroid asteroids[] = {
        {-0.70f, 1.15f, 0.55f, 0.40f},
        { 0.00f, 1.55f, 0.75f, 0.48f},
        { 0.68f, 1.90f, 0.65f, 0.36f},
    };

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1200, 800, "Nave Espacial", NULL, NULL);
    if (!window)
    {
        std::cout << "Error en la inicializacion de la ventana\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

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

    Shader myShader("res/Shader/vertexShader.glsl", "res/Shader/fragmentShader.glsl");

    unsigned int shipVAO;
    unsigned int shipVBO;
    setupBuffer(shipVAO, shipVBO, shipVertices, sizeof(shipVertices));

    unsigned int asteroidVAO;
    unsigned int asteroidVBO;
    setupBuffer(asteroidVAO, asteroidVBO, asteroidVertices, sizeof(asteroidVertices));

    while (!glfwWindowShouldClose(window))
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        const float currentFrame = static_cast<float>(glfwGetTime());
        const float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        const bool turboActive =
            glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;
        const float speedMultiplier = turboActive ? 2.0f : 1.0f;
        const float shipSpeed = normalShipSpeed * speedMultiplier;
        float dirX = 0.0f;
        float dirY = 0.0f;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            dirX += 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            dirX -= 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {
            dirY += 1.0f;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            dirY -= 1.0f;
        }

        shipX += dirX * shipSpeed * deltaTime;
        shipY += dirY * shipSpeed * deltaTime;

        if (shipX > 1.0f - shipHalfWidth)
        {
            shipX = 1.0f - shipHalfWidth;
        }
        if (shipX < -1.0f + shipHalfWidth)
        {
            shipX = -1.0f + shipHalfWidth;
        }
        if (shipY > 1.0f - shipHalfHeight)
        {
            shipY = 1.0f - shipHalfHeight;
        }
        if (shipY < -1.0f + shipHalfHeight)
        {
            shipY = -1.0f + shipHalfHeight;
        }

        for (Asteroid& asteroid : asteroids)
        {
            asteroid.y -= asteroid.speed * normalAsteroidSpeedFactor * speedMultiplier * deltaTime;

            if (asteroid.y + (0.10f * asteroid.scale) < -1.0f)
            {
                asteroid.y = 1.2f;
            }
        }

        glClearColor(0.06f, 0.08f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        myShader.use();

        // Dibuja la nave.
        myShader.setFloat("xOffset", shipX);
        myShader.setFloat("yOffset", shipY);
        myShader.setFloat("scale", 1.0f);
        myShader.setFloat("turboActivo", turboActive ? 1.0f : 0.0f);
        myShader.setFloat("useTurboColor", 1.0f);
        myShader.setFloat("useOverrideColor", 0.0f);
        glBindVertexArray(shipVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Dibuja los asteroides.
        myShader.setFloat("turboActivo", 0.0f);
        myShader.setFloat("useTurboColor", 0.0f);
        myShader.setFloat("useOverrideColor", 1.0f);
        myShader.setFloat("overrideR", 0.55f);
        myShader.setFloat("overrideG", 0.55f);
        myShader.setFloat("overrideB", 0.58f);
        glBindVertexArray(asteroidVAO);

        for (const Asteroid& asteroid : asteroids)
        {
            myShader.setFloat("xOffset", asteroid.x);
            myShader.setFloat("yOffset", asteroid.y);
            myShader.setFloat("scale", asteroid.scale);
            glDrawArrays(GL_TRIANGLES, 0, asteroidVertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &shipVAO);
    glDeleteBuffers(1, &shipVBO);
    glDeleteVertexArrays(1, &asteroidVAO);
    glDeleteBuffers(1, &asteroidVBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
