#ifdef _WIN32
#include <GL/glew.h>
#else
#define GL_SILENCE_DEPRECATION
#include <glad/gl.h>
#endif
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image/stb_image.h"

constexpr unsigned int SCR_WIDTH = 1200;
constexpr unsigned int SCR_HEIGHT = 800;
constexpr float PI = 3.14159265359f;

// Malla base reutilizable
// Esta estructura guarda los buffers y los datos de una malla con triangulos.
// Se reutiliza para la esfera base y evita crear una malla distinta para cada
// planeta, la Luna o el Sol.
struct Mesh {
  unsigned int VAO = 0;
  unsigned int VBO = 0;
  unsigned int EBO = 0;
  std::vector<float> vertices;
  std::vector<unsigned int> indices;
};

// Malla de orbitas
// Esta estructura solo guarda los datos minimos para dibujar los circulos
// guia de las orbitas con lineas.
// No usa normales ni coordenadas UV porque no se ilumina ni se texturiza.
struct OrbitMesh {
  unsigned int VAO = 0;
  unsigned int VBO = 0;
  std::vector<float> vertices;
  int count = 0;
};

// Parametros de cada cuerpo celeste
// Cada objeto de esta estructura define todo lo importante de un cuerpo: su
// tamano visual, su distancia orbital, sus velocidades, su inclinacion y la
// textura que se le aplica.
// parentIndex = -1 significa que el cuerpo orbita directamente al Sol.
// Si apunta a otro cuerpo, la orbita se calcula respecto a ese padre.
struct CelestialBody {
  std::string name;
  float radius;
  float orbitRadius;
  float orbitSpeed;
  float selfRotationSpeed;
  float axialTilt;
  unsigned int textureId;
  int parentIndex;
  bool drawOrbit;
};

// Estado de camara
// Se guarda separado para que el movimiento del observador no se mezcle con la
// logica orbital. yaw y pitch controlan el angulo; distance controla el zoom.
struct CameraState {
  float yaw;
  float pitch;
  float distance;
  glm::vec3 target;
};

// Estado de simulacion
// Aqui viven los controles globales: tiempo acumulado, velocidad, pausa,
// visibilidad de orbitas y el planeta que esta enfocado actualmente.
struct SimulationState {
  float timeValue;
  float timeScale;
  bool paused;
  bool showOrbits;
  int focusedBody;
};

int gWindowWidth = static_cast<int>(SCR_WIDTH);
int gWindowHeight = static_cast<int>(SCR_HEIGHT);

// Funciones auxiliares \n// Estas funciones separan tareas concretas del main:
// cargar texturas, crear mallas, subir datos a GPU, dibujar y liberar recursos
// al final.
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
unsigned int load_texture(const std::string &texturePath);
Mesh create_sphere_mesh(unsigned int sectors, unsigned int stacks);
Mesh create_quad_mesh();
Mesh create_ring_mesh(unsigned int segments, float innerRadius,
                      float outerRadius);
OrbitMesh create_orbit_mesh(unsigned int segments);
void upload_mesh(Mesh &mesh);
void upload_orbit_mesh(OrbitMesh &mesh);
void destroy_mesh(Mesh &mesh);
void destroy_orbit_mesh(OrbitMesh &mesh);
glm::vec3 calculate_camera_position(const CameraState &camera);
glm::vec3 extract_translation(const glm::mat4 &transform);
void draw_mesh(const Mesh &mesh);
void draw_orbit(const OrbitMesh &mesh);

// main
// Aqui se arma toda la escena: ventana, shaders, texturas, mallas, camara y
// lista de cuerpos.
// Si alguien quiere cambiar la composicion general del sistema solar, este es
// el primer bloque que debe revisar.
int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Solar System", nullptr, nullptr);
  if (!window) {
    std::cout << "Error al crear la ventana\n";
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

#ifdef _WIN32
  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK) {
    std::cout << "Error al iniciar GLEW\n";
    glfwTerminate();
    return -1;
  }
#else
  if (!gladLoadGL((GLADloadfunc)glfwGetProcAddress)) {
    std::cout << "Error al iniciar GLAD\n";
    glfwTerminate();
    return -1;
  }
#endif

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  stbi_set_flip_vertically_on_load(true);

  Mesh sphereMesh = create_sphere_mesh(64, 32);
  Mesh quadMesh = create_quad_mesh();
  Mesh ringMesh = create_ring_mesh(96, 1.25f, 1.95f);
  OrbitMesh orbitMesh = create_orbit_mesh(180);

  upload_mesh(sphereMesh);
  upload_mesh(quadMesh);
  upload_mesh(ringMesh);
  upload_orbit_mesh(orbitMesh);

  unsigned int sunTexture = load_texture("res/Texture/sun.jpg");
  unsigned int mercuryTexture = load_texture("res/Texture/mercury.jpg");
  unsigned int venusTexture = load_texture("res/Texture/venus.jpg");
  unsigned int earthTexture = load_texture("res/Texture/earth.jpg");
  unsigned int moonTexture = load_texture("res/Texture/moon.jpg");
  unsigned int marsTexture = load_texture("res/Texture/mars.jpg");
  unsigned int jupiterTexture = load_texture("res/Texture/jupiter.jpg");
  unsigned int saturnTexture = load_texture("res/Texture/saturn.jpg");
  unsigned int uranusTexture = load_texture("res/Texture/uranus.jpg");
  unsigned int neptuneTexture = load_texture("res/Texture/neptune.jpg");
  unsigned int saturnRingTexture = load_texture("res/Texture/saturn_ring.png");
  unsigned int starsTexture = load_texture("res/Texture/stars.jpg");
  unsigned int galacticCoreTexture = load_texture("res/Texture/galactic.png");

  Shader shader("res/Shader/vertexShader.glsl",
                "res/Shader/fragmentShader.glsl");
  shader.use();
  shader.setInt("diffuseTexture", 0);

  std::vector<CelestialBody> bodies = {
      {"Sun", 2.40f, 0.0f, 0.0f, 0.20f, 7.25f, sunTexture, -1, false},
      {"Mercury", 0.28f, 4.0f, 1.60f, 0.40f, 2.0f, mercuryTexture, -1, true},
      {"Venus", 0.42f, 5.6f, 1.20f, 0.18f, 177.0f, venusTexture, -1, true},
      {"Earth", 0.45f, 7.3f, 1.00f, 0.90f, 23.5f, earthTexture, -1, true},
      {"Moon", 0.14f, 0.95f, 3.20f, 0.35f, 6.0f, moonTexture, 3, false},
      {"Mars", 0.34f, 9.0f, 0.82f, 0.75f, 25.0f, marsTexture, -1, true},
      {"Jupiter", 1.10f, 12.8f, 0.45f, 1.20f, 3.0f, jupiterTexture, -1, true},
      {"Saturn", 0.95f, 16.2f, 0.34f, 1.05f, 27.0f, saturnTexture, -1, true},
      {"Uranus", 0.72f, 19.3f, 0.24f, 0.80f, 98.0f, uranusTexture, -1, true},
      {"Neptune", 0.70f, 22.4f, 0.18f, 0.78f, 28.0f, neptuneTexture, -1, true},
  };

  const glm::vec3 orbitColor(0.38f, 0.42f, 0.55f);
  const glm::vec3 galacticOrbitColor(0.72f, 0.60f, 0.24f);
  const float galacticCenterHeight = 45.0f;
  std::vector<glm::mat4> anchors(bodies.size(), glm::mat4(1.0f));
  std::vector<glm::vec3> bodyPositions(bodies.size(), glm::vec3(0.0f));
  std::vector<int> focusBodies = {1, 2, 3, 5, 6, 7, 8, 9, 0};

  const float galacticViewDistance = 72.0f;
  CameraState camera = {0.0f, glm::radians(14.0f), galacticViewDistance,
                        glm::vec3(0.0f)};
  const CameraState defaultCamera = camera;
  SimulationState simulation = {0.0f, 1.0f, false, true, -1};
  const SimulationState defaultSimulation = simulation;

  const float cameraOrbitSpeed = 1.25f;
  const float cameraPitchSpeed = 0.95f;
  const float cameraZoomSpeed = 16.0f;
  const float minCameraDistance = 3.0f;
  const float maxCameraDistance = 90.0f;
  const float maxPitch = glm::radians(70.0f);
  const float minPitch = glm::radians(-25.0f);
  const float minTimeScale = 0.25f;
  const float maxTimeScale = 5.0f;
  const float galacticOrbitRadius = 34.0f;
  const float galacticOrbitSpeed = 0.09f;

  bool pauseWasDown = false;
  bool orbitWasDown = false;
  bool resetWasDown = false;
  bool slowerWasDown = false;
  bool fasterWasDown = false;
  bool originWasDown = false;
  bool focusKeyWasDown[9] = {false};

  float lastFrame = static_cast<float>(glfwGetTime());

  auto key_down = [&](int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
  };

  auto pressed_once = [&](int key, bool &wasDown) {
    bool down = key_down(key);
    bool once = down && !wasDown;
    wasDown = down;
    return once;
  };

  auto pressed_once_either = [&](int firstKey, int secondKey, bool &wasDown) {
    bool down = key_down(firstKey) || key_down(secondKey);
    bool once = down && !wasDown;
    wasDown = down;
    return once;
  };

  auto focus_distance_for_body = [&](int bodyIndex) {
    float distance = bodies[bodyIndex].radius * 2.25f + 1.25f;
    if (distance < minCameraDistance) {
      distance = minCameraDistance;
    }
    if (distance > 9.0f) {
      distance = 9.0f;
    }
    return distance;
  };

  // Bucle principal
  // Este bloque ahora hace cuatro cosas en orden: leer input, actualizar el
  // tiempo, recalcular posiciones orbitales y renderizar la escena completa.
  while (!glfwWindowShouldClose(window)) {
    if (key_down(GLFW_KEY_ESCAPE)) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    float currentFrame = static_cast<float>(glfwGetTime());
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // Controles continuos de camara.
    // A/D o flechas horizontales orbitan la vista; W/S o flechas verticales
    // cambian la inclinacion. Q/E acercan o alejan la camara.
    if (key_down(GLFW_KEY_A) || key_down(GLFW_KEY_LEFT)) {
      camera.yaw -= cameraOrbitSpeed * deltaTime;
    }
    if (key_down(GLFW_KEY_D) || key_down(GLFW_KEY_RIGHT)) {
      camera.yaw += cameraOrbitSpeed * deltaTime;
    }
    if (key_down(GLFW_KEY_W) || key_down(GLFW_KEY_UP)) {
      camera.pitch += cameraPitchSpeed * deltaTime;
    }
    if (key_down(GLFW_KEY_S) || key_down(GLFW_KEY_DOWN)) {
      camera.pitch -= cameraPitchSpeed * deltaTime;
    }
    if (key_down(GLFW_KEY_Q)) {
      camera.distance -= cameraZoomSpeed * deltaTime;
    }
    if (key_down(GLFW_KEY_E)) {
      camera.distance += cameraZoomSpeed * deltaTime;
    }

    if (camera.pitch > maxPitch) {
      camera.pitch = maxPitch;
    }
    if (camera.pitch < minPitch) {
      camera.pitch = minPitch;
    }
    if (camera.distance < minCameraDistance) {
      camera.distance = minCameraDistance;
    }
    if (camera.distance > maxCameraDistance) {
      camera.distance = maxCameraDistance;
    }

    // Controles por pulsacion unica.
    // Space pausa el sistema, O oculta orbitas, R restablece la vista,
    // +/- cambian la velocidad y 0-8 cambian el foco de la camara.
    if (pressed_once(GLFW_KEY_SPACE, pauseWasDown)) {
      simulation.paused = !simulation.paused;
    }
    if (pressed_once(GLFW_KEY_O, orbitWasDown)) {
      simulation.showOrbits = !simulation.showOrbits;
    }
    if (pressed_once(GLFW_KEY_R, resetWasDown)) {
      camera = defaultCamera;
      simulation = defaultSimulation;
    }
    if (pressed_once_either(GLFW_KEY_MINUS, GLFW_KEY_KP_SUBTRACT,
                            slowerWasDown)) {
      simulation.timeScale -= 0.25f;
      if (simulation.timeScale < minTimeScale) {
        simulation.timeScale = minTimeScale;
      }
    }
    if (pressed_once_either(GLFW_KEY_EQUAL, GLFW_KEY_KP_ADD, fasterWasDown)) {
      simulation.timeScale += 0.25f;
      if (simulation.timeScale > maxTimeScale) {
        simulation.timeScale = maxTimeScale;
      }
    }
    if (pressed_once(GLFW_KEY_0, originWasDown)) {
      simulation.focusedBody = -1;
      camera.distance = galacticViewDistance;
    }

    for (std::size_t i = 0; i < focusBodies.size(); ++i) {
      int key = GLFW_KEY_1 + static_cast<int>(i);
      if (pressed_once(key, focusKeyWasDown[i])) {
        simulation.focusedBody = focusBodies[i];
        camera.distance = focus_distance_for_body(simulation.focusedBody);
      }
    }

    if (!simulation.paused) {
      simulation.timeValue += deltaTime * simulation.timeScale;
    }

    // Precalculo orbital.
    // Primero se guardan las transformaciones base de cada cuerpo para
    // reutilizar esa informacion tanto en el enfoque de camara como en el
    // render.
    float galacticAngle = simulation.timeValue * galacticOrbitSpeed;
    glm::vec3 sunPosition(std::cos(galacticAngle) * galacticOrbitRadius, 0.0f,
                          std::sin(galacticAngle) * galacticOrbitRadius);
    glm::mat4 solarSystemAnchor =
        glm::translate(glm::mat4(1.0f), sunPosition);

    for (std::size_t i = 0; i < bodies.size(); ++i) {
      const CelestialBody &body = bodies[i];
      glm::mat4 anchor = glm::mat4(1.0f);

      if (i == 0) {
        anchor = solarSystemAnchor;
      } else if (body.parentIndex >= 0) {
        anchor = anchors[body.parentIndex];
      } else {
        anchor = solarSystemAnchor;
      }

      if (body.orbitRadius > 0.0f) {
        anchor = glm::rotate(anchor, simulation.timeValue * body.orbitSpeed,
                             glm::vec3(0.0f, 1.0f, 0.0f));
        anchor =
            glm::translate(anchor, glm::vec3(body.orbitRadius, 0.0f, 0.0f));
      }

      anchors[i] = anchor;
      bodyPositions[i] = extract_translation(anchor);
    }

    // Si hay un planeta enfocado, la camara sigue ese cuerpo. Si no, observa el
    // centro del sistema completo.
    if (simulation.focusedBody >= 0) {
      camera.target = bodyPositions[simulation.focusedBody];
    } else {
      camera.target = glm::vec3(0.0f);
    }

    glm::vec3 cameraPos = calculate_camera_position(camera);
    float aspect =
        static_cast<float>(gWindowWidth) / static_cast<float>(gWindowHeight);
    glm::mat4 projection =
        glm::perspective(glm::radians(45.0f), aspect, 0.1f, 120.0f);
    glm::mat4 view =
        glm::lookAt(cameraPos, camera.target, glm::vec3(0.0f, 1.0f, 0.0f));

    glClearColor(0.01f, 0.01f, 0.04f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("lightPos", sunPosition.x, sunPosition.y, sunPosition.z);
    shader.setVec3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

    glDepthMask(GL_FALSE);
    shader.setBool("useTexture", true);
    shader.setBool("useLighting", false);
    shader.setFloat("alpha", 1.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, starsTexture);
    glm::mat4 starsModel = glm::translate(glm::mat4(1.0f), cameraPos);
    starsModel = glm::scale(starsModel, glm::vec3(70.0f));
    shader.setMat4("model", starsModel);
    draw_mesh(sphereMesh);
    glDepthMask(GL_TRUE);

    shader.setBool("useTexture", true);
    shader.setBool("useLighting", false);
    glBindTexture(GL_TEXTURE_2D, galacticCoreTexture);
    glDepthMask(GL_FALSE);

    glm::mat4 galacticCenterModel =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f));
    galacticCenterModel *= glm::mat4(glm::mat3(glm::inverse(view)));
    galacticCenterModel = glm::scale(
        galacticCenterModel,
        glm::vec3(galacticCenterHeight, galacticCenterHeight, 1.0f));
    shader.setFloat("alpha", 1.0f);
    shader.setMat4("model", galacticCenterModel);
    draw_mesh(quadMesh);
    glDepthMask(GL_TRUE);

    if (simulation.showOrbits) {
      shader.setBool("useTexture", false);
      shader.setBool("useLighting", false);
      shader.setVec3("baseColor", galacticOrbitColor.x, galacticOrbitColor.y,
                     galacticOrbitColor.z);
      shader.setFloat("alpha", 0.8f);

      glm::mat4 galacticOrbitModel =
          glm::scale(glm::mat4(1.0f), glm::vec3(galacticOrbitRadius));
      shader.setMat4("model", galacticOrbitModel);
      draw_orbit(orbitMesh);

      shader.setVec3("baseColor", orbitColor.x, orbitColor.y, orbitColor.z);
      shader.setFloat("alpha", 0.65f);

      for (const CelestialBody &body : bodies) {
        if (!body.drawOrbit || body.orbitRadius <= 0.0f) {
          continue;
        }

        glm::vec3 orbitCenter =
            body.parentIndex >= 0 ? bodyPositions[body.parentIndex]
                                  : sunPosition;
        glm::mat4 orbitModel =
            glm::translate(glm::mat4(1.0f), orbitCenter);
        orbitModel = glm::scale(orbitModel, glm::vec3(body.orbitRadius));
        shader.setMat4("model", orbitModel);
        draw_orbit(orbitMesh);
      }
    }

    for (std::size_t i = 0; i < bodies.size(); ++i) {
      const CelestialBody &body = bodies[i];
      glm::mat4 model = anchors[i];
      model = glm::rotate(model, glm::radians(body.axialTilt),
                          glm::vec3(0.0f, 0.0f, 1.0f));
      model = glm::rotate(model, simulation.timeValue * body.selfRotationSpeed,
                          glm::vec3(0.0f, 1.0f, 0.0f));
      model = glm::scale(model, glm::vec3(body.radius));

      shader.setBool("useTexture", true);
      shader.setBool("useLighting", body.name != "Sun");
      shader.setFloat("alpha", 1.0f);
      glBindTexture(GL_TEXTURE_2D, body.textureId);
      shader.setMat4("model", model);
      draw_mesh(sphereMesh);

      if (body.name == "Saturn") {
        glm::mat4 ringModel = anchors[i];
        ringModel = glm::rotate(ringModel, glm::radians(body.axialTilt),
                                glm::vec3(0.0f, 0.0f, 1.0f));
        ringModel = glm::rotate(
            ringModel, simulation.timeValue * body.selfRotationSpeed * 0.35f,
            glm::vec3(0.0f, 1.0f, 0.0f));
        ringModel = glm::scale(ringModel, glm::vec3(body.radius));

        shader.setBool("useLighting", false);
        glBindTexture(GL_TEXTURE_2D, saturnRingTexture);
        shader.setMat4("model", ringModel);
        draw_mesh(ringMesh);
      }
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  destroy_mesh(sphereMesh);
  destroy_mesh(quadMesh);
  destroy_mesh(ringMesh);
  // Limpieza final: libera los buffers creados en GPU antes de cerrar.
  destroy_orbit_mesh(orbitMesh);

  glDeleteTextures(1, &sunTexture);
  glDeleteTextures(1, &mercuryTexture);
  glDeleteTextures(1, &venusTexture);
  glDeleteTextures(1, &earthTexture);
  glDeleteTextures(1, &moonTexture);
  glDeleteTextures(1, &marsTexture);
  glDeleteTextures(1, &jupiterTexture);
  glDeleteTextures(1, &saturnTexture);
  glDeleteTextures(1, &uranusTexture);
  glDeleteTextures(1, &neptuneTexture);
  glDeleteTextures(1, &saturnRingTexture);
  glDeleteTextures(1, &starsTexture);
  glDeleteTextures(1, &galacticCoreTexture);

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  gWindowWidth = width;
  gWindowHeight = height > 0 ? height : 1;
  glViewport(0, 0, width, height);
}

// Posicion de camara
// Convierte yaw, pitch y distancia en una posicion 3D real alrededor del punto
// que se esta observando. Asi la camara siempre orbita el objetivo actual.
glm::vec3 calculate_camera_position(const CameraState &camera) {
  float cosPitch = std::cos(camera.pitch);
  return camera.target +
         glm::vec3(camera.distance * std::sin(camera.yaw) * cosPitch,
                   camera.distance * std::sin(camera.pitch),
                   camera.distance * std::cos(camera.yaw) * cosPitch);
}

// Extraer traslacion
// Cada anchor guarda rotacion y traslacion. Aqui solo tomamos la posicion final
// del cuerpo para poder enfocar la camara o reutilizar el dato luego.
glm::vec3 extract_translation(const glm::mat4 &transform) {
  return glm::vec3(transform[3][0], transform[3][1], transform[3][2]);
}

unsigned int load_texture(const std::string &texturePath) {
  unsigned int texture = 0;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width = 0;
  int height = 0;
  int channels = 0;
  unsigned char *data =
      stbi_load(texturePath.c_str(), &width, &height, &channels, 0);

  if (!data) {
    std::cout << "No se pudo cargar la textura: " << texturePath << "\n";
    return texture;
  }

  GLenum format = GL_RGB;
  if (channels == 1) {
    format = GL_RED;
  } else if (channels == 3) {
    format = GL_RGB;
  } else if (channels == 4) {
    format = GL_RGBA;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
               GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);
  stbi_image_free(data);
  return texture;
}

Mesh create_sphere_mesh(unsigned int sectors, unsigned int stacks) {
  Mesh mesh;

  for (unsigned int stack = 0; stack <= stacks; ++stack) {
    float stackAngle =
        PI * 0.5f - static_cast<float>(stack) * PI / static_cast<float>(stacks);
    float xy = std::cos(stackAngle);
    float y = std::sin(stackAngle);

    for (unsigned int sector = 0; sector <= sectors; ++sector) {
      float sectorAngle =
          static_cast<float>(sector) * 2.0f * PI / static_cast<float>(sectors);
      float x = xy * std::cos(sectorAngle);
      float z = xy * std::sin(sectorAngle);
      float u = static_cast<float>(sector) / static_cast<float>(sectors);
      float v = 1.0f - static_cast<float>(stack) / static_cast<float>(stacks);

      mesh.vertices.push_back(x);
      mesh.vertices.push_back(y);
      mesh.vertices.push_back(z);
      mesh.vertices.push_back(x);
      mesh.vertices.push_back(y);
      mesh.vertices.push_back(z);
      mesh.vertices.push_back(u);
      mesh.vertices.push_back(v);
    }
  }

  for (unsigned int stack = 0; stack < stacks; ++stack) {
    unsigned int k1 = stack * (sectors + 1);
    unsigned int k2 = k1 + sectors + 1;

    for (unsigned int sector = 0; sector < sectors; ++sector) {
      if (stack != 0) {
        mesh.indices.push_back(k1 + sector);
        mesh.indices.push_back(k2 + sector);
        mesh.indices.push_back(k1 + sector + 1);
      }

      if (stack != stacks - 1) {
        mesh.indices.push_back(k1 + sector + 1);
        mesh.indices.push_back(k2 + sector);
        mesh.indices.push_back(k2 + sector + 1);
      }
    }
  }

  return mesh;
}

Mesh create_quad_mesh() {
  Mesh mesh;
  mesh.vertices = {
      -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
       0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
       0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
      -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
  };
  mesh.indices = {0, 1, 2, 2, 3, 0};
  return mesh;
}

Mesh create_ring_mesh(unsigned int segments, float innerRadius,
                      float outerRadius) {
  Mesh mesh;

  for (unsigned int i = 0; i <= segments; ++i) {
    float angle =
        static_cast<float>(i) * 2.0f * PI / static_cast<float>(segments);
    float c = std::cos(angle);
    float s = std::sin(angle);

    float outerX = c * outerRadius;
    float outerZ = s * outerRadius;
    mesh.vertices.push_back(outerX);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(outerZ);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(1.0f);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(c * 0.5f + 0.5f);
    mesh.vertices.push_back(s * 0.5f + 0.5f);

    float innerX = c * innerRadius;
    float innerZ = s * innerRadius;
    mesh.vertices.push_back(innerX);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(innerZ);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(1.0f);
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(c * 0.5f * (innerRadius / outerRadius) + 0.5f);
    mesh.vertices.push_back(s * 0.5f * (innerRadius / outerRadius) + 0.5f);
  }

  for (unsigned int i = 0; i < segments; ++i) {
    unsigned int current = i * 2;
    unsigned int next = current + 2;

    mesh.indices.push_back(current);
    mesh.indices.push_back(current + 1);
    mesh.indices.push_back(next);

    mesh.indices.push_back(next);
    mesh.indices.push_back(current + 1);
    mesh.indices.push_back(next + 1);
  }

  return mesh;
}

OrbitMesh create_orbit_mesh(unsigned int segments) {
  OrbitMesh mesh;
  mesh.count = static_cast<int>(segments);

  for (unsigned int i = 0; i < segments; ++i) {
    float angle =
        static_cast<float>(i) * 2.0f * PI / static_cast<float>(segments);
    mesh.vertices.push_back(std::cos(angle));
    mesh.vertices.push_back(0.0f);
    mesh.vertices.push_back(std::sin(angle));
  }

  return mesh;
}

void upload_mesh(Mesh &mesh) {
  glGenVertexArrays(1, &mesh.VAO);
  glGenBuffers(1, &mesh.VBO);
  glGenBuffers(1, &mesh.EBO);

  glBindVertexArray(mesh.VAO);

  glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
  glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float),
               mesh.vertices.data(), GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               mesh.indices.size() * sizeof(unsigned int), mesh.indices.data(),
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);
}

void upload_orbit_mesh(OrbitMesh &mesh) {
  glGenVertexArrays(1, &mesh.VAO);
  glGenBuffers(1, &mesh.VBO);

  glBindVertexArray(mesh.VAO);
  glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
  glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float),
               mesh.vertices.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
}

void destroy_mesh(Mesh &mesh) {
  glDeleteVertexArrays(1, &mesh.VAO);
  glDeleteBuffers(1, &mesh.VBO);
  glDeleteBuffers(1, &mesh.EBO);
}

void destroy_orbit_mesh(OrbitMesh &mesh) {
  glDeleteVertexArrays(1, &mesh.VAO);
  glDeleteBuffers(1, &mesh.VBO);
}

void draw_mesh(const Mesh &mesh) {
  glBindVertexArray(mesh.VAO);
  glDrawElements(GL_TRIANGLES, static_cast<int>(mesh.indices.size()),
                 GL_UNSIGNED_INT, nullptr);
}

void draw_orbit(const OrbitMesh &mesh) {
  glBindVertexArray(mesh.VAO);
  glDrawArrays(GL_LINE_LOOP, 0, mesh.count);
}
