/*
 * LSystem Forest — Tarea 1, Computación Gráfica y Visual
 *
 * Genera un bosque 2D usando L-Systems (Lindenmayer Systems).
 * Cada árbol tiene una gramática propia que define la forma de sus ramas.
 * El bosque tiene 4 capas de profundidad con perspectiva atmosférica.
 * Los arbustos de la base se animan usando sin(tiempo).
 *
 * Técnicas:
 *   - L-System: reescritura de cadenas → geometría de ramas
 *   - Tortuga (turtle graphics): interpreta la cadena como instrucciones de dibujo
 *   - Algoritmo del pintor: dibuja de atrás hacia adelante para simular profundidad
 *   - Perspectiva atmosférica: capas lejanas con colores más lavados
 */

#ifdef _WIN32
#include <GL/glew.h>
#elif defined(__APPLE__)
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <glad/glad.h>
#endif

#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include "Shader.h"

static constexpr float PI = 3.14159265f;

// ─── Estructuras ──────────────────────────────────────────────────────────────

struct Punto { float x, y; };

// profundidad: qué tan adentro del árbol está el segmento (0 = tronco/raíz)
struct Segmento { Punto desde, hasta; int profundidad; };

struct EstadoTortuga { float x, y, angulo; };

struct ArbolParams {
    float baseX;         // posición X del árbol en pantalla (-1 a 1)
    float suelo;         // Y donde comienza el tronco
    float alturaTronco;  // qué tan alto es el tronco
    float anchoTronco;   // grosor del tronco
    float largoRama;     // longitud de cada segmento F
    float angulo;        // grados que gira la tortuga con + o -
    float jitter;        // variación aleatoria del ángulo (naturalidad)
    int   iteraciones;   // cuántas veces se expande la gramática
    int   capa;          // profundidad: 3=muy fondo  0=fondo  1=medio  2=frente
    unsigned int semilla;// semilla para la variación aleatoria (cada árbol es único)
    const char* axiom;   // símbolo inicial del L-System
    const char* reglaF;  // qué produce F al expandirse
    const char* reglaX;  // qué produce X (opcional, "" si no se usa)
};

struct Arbusto { float cx, cy, radio, fase, vel; };

// ─── Utilidades ───────────────────────────────────────────────────────────────

void framebufferSizeCallback(GLFWwindow* w, int ancho, int alto)
{ (void)w; glViewport(0, 0, ancho, alto); }

void procesarInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

float rnd(unsigned int& s)
{
    s = s * 1664525u + 1013904223u;
    return static_cast<float>(s & 0x00FFFFFFu) / static_cast<float>(0x00FFFFFFu);
}

void agregarTriangulo(std::vector<float>& v,
                      float x1, float y1, float x2, float y2, float x3, float y3,
                      float r, float g, float b)
{
    v.insert(v.end(), { x1,y1,0.f,r,g,b, x2,y2,0.f,r,g,b, x3,y3,0.f,r,g,b });
}

void agregarTrapecio(std::vector<float>& v,
                     float bxIzq, float bxDer, float by,
                     float txIzq, float txDer, float ty,
                     float r, float g, float b)
{
    agregarTriangulo(v, bxIzq,by, bxDer,by, txDer,ty, r,g,b);
    agregarTriangulo(v, bxIzq,by, txDer,ty, txIzq,ty, r,g,b);
}

void agregarLinea(std::vector<float>& v,
                  float x1, float y1, float x2, float y2,
                  float hw, float r, float g, float b)
{
    float dx = x2-x1, dy = y2-y1;
    float len = std::sqrt(dx*dx + dy*dy);
    if (len < 0.0001f) return;
    float nx = -dy/len * hw, ny = dx/len * hw;
    agregarTriangulo(v, x1-nx,y1-ny, x1+nx,y1+ny, x2+nx,y2+ny, r,g,b);
    agregarTriangulo(v, x1-nx,y1-ny, x2+nx,y2+ny, x2-nx,y2-ny, r,g,b);
}

void agregarCirculo(std::vector<float>& v, float cx, float cy, float radio,
                    float r, float g, float b, int lados = 10)
{
    for (int i = 0; i < lados; ++i) {
        float a0 = (float)i       / lados * 2.0f * PI;
        float a1 = (float)(i + 1) / lados * 2.0f * PI;
        agregarTriangulo(v,
            cx,                        cy,
            cx + std::cos(a0) * radio, cy + std::sin(a0) * radio,
            cx + std::cos(a1) * radio, cy + std::sin(a1) * radio,
            r, g, b);
    }
}

// ─── Motor L-System ───────────────────────────────────────────────────────────
// Expande el axiom aplicando las reglas el número de iteraciones dado.
// Ejemplo con F→F[+F][-F]F, iter=2:
//   iter 0: "F"
//   iter 1: "F[+F][-F]F"
//   iter 2: "F[+F][-F]F[+F[+F][-F]F][-F[+F][-F]F]F[+F][-F]F"
// Esta función solo genera la cadena. No sabe nada de dibujo.
std::string generarLSystem(const std::string& axiom,
                           const std::unordered_map<char,std::string>& reglas,
                           int iteraciones)
{
    std::string actual = axiom;
    for (int i = 0; i < iteraciones; ++i) {
        std::string siguiente;
        siguiente.reserve(actual.size() * 5);
        for (char c : actual) {
            auto it = reglas.find(c);
            siguiente += (it != reglas.end()) ? it->second : std::string(1, c);
        }
        actual = std::move(siguiente);
    }
    return actual;
}

// ─── Intérprete tortuga ───────────────────────────────────────────────────────
// Lee la cadena del L-System carácter por carácter y mueve una "tortuga"
// que deja rastro de segmentos (ramas). Tabla de símbolos:
//   F  → avanza y dibuja un segmento de rama
//   +  → gira a la izquierda (ángulo positivo)
//   -  → gira a la derecha (ángulo negativo)
//   [  → guarda posición y ángulo actual (inicio de sub-rama)
//   ]  → recupera posición y ángulo guardados (fin de sub-rama)
// La tortuga arranca en la cima del tronco mirando hacia arriba (90°).
// Se rastrea la profundidad de brackets para saber el grosor de cada rama.
std::vector<Segmento> interpretarTortuga(const std::string& cadena,
                                         const ArbolParams& arbol)
{
    std::vector<Segmento> segmentos;
    std::stack<EstadoTortuga> pila;
    EstadoTortuga estado = { arbol.baseX, arbol.suelo + arbol.alturaTronco, 90.0f };
    unsigned int s = arbol.semilla;
    int profundidad = 0;

    for (char c : cadena) {
        if (c == 'F') {
            float rad = estado.angulo * PI / 180.0f;
            float lon = arbol.largoRama * (0.85f + rnd(s) * 0.28f);
            Punto nuevo = { estado.x + std::cos(rad)*lon, estado.y + std::sin(rad)*lon };
            segmentos.push_back({ {estado.x, estado.y}, nuevo, profundidad });
            estado.x = nuevo.x;
            estado.y = nuevo.y;
        }
        else if (c == '+') estado.angulo += arbol.angulo + (rnd(s)-0.5f)*arbol.jitter;
        else if (c == '-') estado.angulo -= arbol.angulo + (rnd(s)-0.5f)*arbol.jitter;
        else if (c == '[') { pila.push(estado); profundidad++; }
        else if (c == ']') { if (!pila.empty()) { estado = pila.top(); pila.pop(); profundidad--; } }
        // letras desconocidas (X, A…) = variables sin dibujo, se ignoran
    }
    return segmentos;
}

// ─── Constructor de árbol ─────────────────────────────────────────────────────

void construirArbol(const ArbolParams& a,
                    std::vector<float>& verticesTronco,
                    std::vector<float>& verticesRamas,
                    std::vector<float>& verticesHojas)
{
    // Colores de tronco por capa (perspectiva atmosférica: más lejano = más lavado)
    float tR = (a.capa==3) ? 0.55f : (a.capa==0) ? 0.47f : (a.capa==1) ? 0.41f : 0.35f;
    float tG = (a.capa==3) ? 0.40f : (a.capa==0) ? 0.31f : (a.capa==1) ? 0.25f : 0.19f;
    float tB = (a.capa==3) ? 0.24f : (a.capa==0) ? 0.16f : (a.capa==1) ? 0.11f : 0.07f;

    // Tronco: base ensanchada + columna que se afina
    float ensanche    = a.anchoTronco * 1.35f;
    float altEnsanche = a.alturaTronco * 0.22f;
    agregarTrapecio(verticesTronco,
        a.baseX - ensanche,              a.baseX + ensanche,              a.suelo,
        a.baseX - a.anchoTronco * 0.45f, a.baseX + a.anchoTronco * 0.45f, a.suelo + altEnsanche,
        tR*0.80f, tG*0.80f, tB*0.80f);
    agregarTrapecio(verticesTronco,
        a.baseX - a.anchoTronco * 0.45f, a.baseX + a.anchoTronco * 0.45f, a.suelo + altEnsanche,
        a.baseX - a.anchoTronco * 0.16f, a.baseX + a.anchoTronco * 0.16f, a.suelo + a.alturaTronco,
        tR, tG, tB);

    // Generar cadena L-System
    std::unordered_map<char,std::string> reglas;
    reglas['F'] = a.reglaF;
    if (a.reglaX[0] != '\0') reglas['X'] = a.reglaX;
    const std::string cadena = generarLSystem(a.axiom, reglas, a.iteraciones);
    const std::vector<Segmento> segs = interpretarTortuga(cadena, a);

    // hwBase derivado del anchoTronco → la primera rama arranca con grosor
    // compatible con la cima del tronco, sin salto visual brusco
    float hwBase = a.anchoTronco * 0.22f;

    // Colores de rama (ligeramente más claros que tronco)
    float bR = tR + 0.05f, bG = tG + 0.03f, bB = tB + 0.02f;

    // Colores de hoja por capa
    float hR = (a.capa==3) ? 0.38f : (a.capa==0) ? 0.28f : (a.capa==1) ? 0.20f : 0.14f;
    float hG = (a.capa==3) ? 0.62f : (a.capa==0) ? 0.54f : (a.capa==1) ? 0.49f : 0.43f;
    float hB = (a.capa==3) ? 0.28f : (a.capa==0) ? 0.18f : (a.capa==1) ? 0.12f : 0.08f;

    unsigned int s = a.semilla + 88888u;

    for (const Segmento& seg : segs) {
        // Grosor decrece gradualmente con la profundidad (0.70 por nivel)
        // prof=0 → rama principal  prof=1 → 70%  prof=2 → 49%  prof=3 → 34%
        float hw = hwBase * std::pow(0.70f, (float)seg.profundidad);
        hw = std::max(hw, 0.0003f);
        agregarLinea(verticesRamas,
                     seg.desde.x, seg.desde.y,
                     seg.hasta.x, seg.hasta.y,
                     hw, bR, bG, bB);

        // Círculo de follaje en el extremo de cada sub-rama (prof >= 1)
        if (seg.profundidad >= 1) {
            float radio = a.largoRama * (0.7f + rnd(s) * 1.0f);
            float rv = hR + (rnd(s)-0.5f) * 0.07f;
            float gv = hG + (rnd(s)-0.5f) * 0.09f;
            float bv = hB + (rnd(s)-0.5f) * 0.04f;
            agregarCirculo(verticesHojas, seg.hasta.x, seg.hasta.y, radio, rv, gv, bv);
        }
    }
}

// ─── Arbustos animados ────────────────────────────────────────────────────────

void construirArbustos(std::vector<float>& verts,
                       const std::vector<Arbusto>& arbustos,
                       float tiempo)
{
    verts.clear();
    for (const Arbusto& a : arbustos) {
        // Todos oscilan al mismo ritmo base, desfasados (ritmo compartido)
        float swing = std::sin(tiempo * 1.1f + a.fase) * 0.007f;
        float r = 0.18f + std::fmod(a.fase, 0.8f) * 0.04f;
        float g = 0.46f + std::fmod(a.fase, 0.6f) * 0.04f;
        float b = 0.10f;
        agregarCirculo(verts, a.cx + swing, a.cy, a.radio, r, g, b, 12);
    }
}

// ─── Carga en GPU ─────────────────────────────────────────────────────────────

void subirBuffer(unsigned int& vao, unsigned int& vbo,
                 const std::vector<float>& verts, GLenum uso)
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 (GLsizeiptr)(verts.size()*sizeof(float)),
                 verts.data(), uso);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float),
                          (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
}

void dibujar(unsigned int vao, int nVerts)
{
    if (nVerts <= 0) return;
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLES, 0, nVerts);
}

// ─── main ─────────────────────────────────────────────────────────────────────

int main()
{
    if (!glfwInit()) { std::cout << "Error GLFW\n"; return -1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(1400, 900, "LSystem – Bosque", nullptr, nullptr);
    if (!window) { std::cout << "Error ventana\n"; glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

#ifdef _WIN32
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) { glfwTerminate(); return -1; }
#endif

    Shader shader("res/Shader/vertexShader.glsl", "res/Shader/fragmentShader.glsl");

    // ─────────────────────────────────────────────────────────────────────────
    // Definición de los árboles del bosque.
    // Todos son del mismo tipo (misma familia, ángulos 20-27°) pero cada uno
    // tiene una gramática L-System diferente → formas estructuralmente distintas,
    // como ocurre en la naturaleza con árboles de la misma especie.
    //
    // Columnas: baseX | suelo | alturaTronco | anchoTronco | largoRama |
    //           angulo | jitter | iteraciones | capa | semilla | axiom | reglaF | reglaX
    //
    // Gramáticas usadas (cada una produce una estructura diferente):
    //   F→F[+F][-F]F              bifurcación binaria simple
    //   F→F[+F][-F][F]            trifurcación: 3 sub-ramas por nodo
    //   F→F[+F]F[-F]F             5 pasos por nodo, copa más densa
    //   F→F[+FF][-FF]F            brazos dobles laterales (candelabro)
    //   F→F[+F[+F][-F]][-F[+F][-F]]   sub-ramas anidadas
    //   F→F[+F[+F]][-F[-F]]F     anidadas en la misma dirección
    //   F→FF+[+F-F-F]-[-F+F+F]   gramática del ejemplo de clase
    //   F→FF-[-F+F+F]+[+F-F-F]   espejo del ejemplo de clase
    // ─────────────────────────────────────────────────────────────────────────
    const ArbolParams arboles[] = {

        // ── Capa muy fondo (3) – laterales + centro ──────────────────────────
        { -0.96f,-0.92f, 0.09f,0.004f, 0.015f, 23.0f,2.0f, 2,3,5151u,
          "F","F[+F][-F]F","" },
        { -0.88f,-0.92f, 0.10f,0.004f, 0.016f, 24.0f,2.0f, 2,3,1111u,
          "F","F[+F][-F]F","" },
        { -0.40f,-0.92f, 0.11f,0.004f, 0.017f, 22.0f,2.5f, 2,3,2222u,
          "F","F[+F][-F][F]","" },
        {  0.08f,-0.92f, 0.10f,0.004f, 0.016f, 26.0f,2.0f, 2,3,3333u,
          "F","F[+F]F[-F]F","" },
        {  0.55f,-0.92f, 0.11f,0.004f, 0.017f, 23.0f,2.5f, 2,3,4444u,
          "F","F[+FF][-FF]F","" },
        {  0.88f,-0.92f, 0.10f,0.004f, 0.016f, 25.0f,2.0f, 2,3,9999u,
          "F","F[+F[+F][-F]][-F[+F][-F]]","" },
        {  0.96f,-0.92f, 0.09f,0.004f, 0.015f, 24.0f,2.0f, 2,3,6262u,
          "F","F[+F][-F][F]","" },

        // ── Capa fondo (0) – laterales + interior ────────────────────────────
        { -0.92f,-0.92f, 0.16f,0.006f, 0.020f, 25.0f,2.5f, 3,0,8421u,
          "F","F[+F][-F][F]","" },
        { -0.76f,-0.92f, 0.17f,0.007f, 0.022f, 25.0f,2.5f, 3,0,1011u,
          "F","F[+F][-F]F","" },
        { -0.64f,-0.92f, 0.16f,0.006f, 0.021f, 23.0f,2.5f, 3,0,7531u,
          "F","F[+F][-F][F]","" },
        { -0.18f,-0.92f, 0.19f,0.007f, 0.023f, 22.0f,2.0f, 3,0,3737u,
          "F","F[+F[+F][-F]][-F[+F][-F]]","" },
        {  0.28f,-0.92f, 0.18f,0.006f, 0.021f, 27.0f,3.0f, 3,0,6666u,
          "F","F[+F[+F]][-F[-F]]F","" },
        {  0.70f,-0.92f, 0.17f,0.006f, 0.022f, 26.0f,2.5f, 3,0,5555u,
          "F","F[+F]F[-F]F","" },
        {  0.84f,-0.92f, 0.16f,0.006f, 0.020f, 24.0f,2.5f, 3,0,1357u,
          "F","F[+F][-F]F","" },

        // ── Capa media (1) – laterales + interior ────────────────────────────
        { -0.82f,-0.92f, 0.26f,0.009f, 0.045f, 21.0f,2.5f, 2,1,4242u,
          "F","F[+F][-F]F","" },
        { -0.55f,-0.92f, 0.27f,0.010f, 0.048f, 22.0f,2.5f, 2,1,7777u,
          "F","F[+FF][-FF]F","" },
        { -0.02f,-0.92f, 0.25f,0.009f, 0.044f, 24.0f,2.0f, 2,1,9191u,
          "F","F[+F[+F][-F]][-F[+F][-F]]","" },
        {  0.32f,-0.92f, 0.26f,0.009f, 0.045f, 20.0f,2.5f, 2,1,2424u,
          "F","F[+F[+F]][-F[-F]]F","" },
        {  0.46f,-0.92f, 0.25f,0.009f, 0.044f, 23.0f,2.0f, 2,1,3131u,
          "F","F[+F][-F]F","" },
        {  0.60f,-0.92f, 0.24f,0.009f, 0.043f, 22.0f,2.0f, 2,1,8888u,
          "F","F[+FF][-FF]F","" },
        {  0.76f,-0.92f, 0.26f,0.009f, 0.045f, 21.0f,2.5f, 2,1,2468u,
          "F","F[+F[+F][-F]][-F[+F][-F]]","" },

        // ── Capa frente (2) – 2 árboles grandes ──────────────────────────────
        { -0.26f,-0.92f, 0.40f,0.014f, 0.030f, 22.5f,2.5f, 2,2,4848u,
          "F","FF+[+F-F-F]-[-F+F+F]","" },
        {  0.36f,-0.92f, 0.42f,0.013f, 0.030f, 22.0f,2.5f, 2,2,6060u,
          "F","FF-[-F+F+F]+[+F-F-F]","" },
    };

    // ─── Arbustos en la base ──────────────────────────────────────────────────
    // Franja de 18 arbustos que cubre todo el ancho. Cada uno tiene su fase
    // para que el balanceo se vea natural (no todos juntos al mismo tiempo).
    const std::vector<Arbusto> arbustos = {
        {-1.00f,-0.89f,0.090f,0.00f,1.0f}, {-0.87f,-0.90f,0.080f,0.60f,1.1f},
        {-0.74f,-0.89f,0.085f,1.20f,0.9f}, {-0.61f,-0.90f,0.078f,1.80f,1.2f},
        {-0.48f,-0.89f,0.088f,2.40f,1.0f}, {-0.35f,-0.90f,0.082f,3.00f,1.1f},
        {-0.22f,-0.89f,0.087f,3.60f,0.9f}, {-0.09f,-0.90f,0.080f,4.20f,1.0f},
        { 0.04f,-0.89f,0.083f,4.80f,1.2f}, { 0.17f,-0.90f,0.079f,5.40f,1.0f},
        { 0.30f,-0.89f,0.086f,0.30f,0.9f}, { 0.43f,-0.90f,0.081f,0.90f,1.1f},
        { 0.56f,-0.89f,0.084f,1.50f,1.0f}, { 0.69f,-0.90f,0.078f,2.10f,1.2f},
        { 0.82f,-0.89f,0.087f,2.70f,0.9f}, { 0.95f,-0.90f,0.082f,3.30f,1.0f},
        {-0.93f,-0.91f,0.070f,1.00f,1.1f}, { 0.88f,-0.91f,0.072f,4.00f,0.9f},
    };

    // ─── Construir geometría ──────────────────────────────────────────────────
    std::vector<float> trMuyFondo, raMuyFondo, hjMuyFondo;
    std::vector<float> trFondo,    raFondo,    hjFondo;
    std::vector<float> trMedio,    raMedio,    hjMedio;
    std::vector<float> trFrente,   raFrente,   hjFrente;

    for (const ArbolParams& a : arboles) {
        std::vector<float> tronco, ramas, hojas;
        construirArbol(a, tronco, ramas, hojas);
        auto& vT = (a.capa==3) ? trMuyFondo : (a.capa==0) ? trFondo : (a.capa==1) ? trMedio : trFrente;
        auto& vR = (a.capa==3) ? raMuyFondo : (a.capa==0) ? raFondo : (a.capa==1) ? raMedio : raFrente;
        auto& vH = (a.capa==3) ? hjMuyFondo : (a.capa==0) ? hjFondo : (a.capa==1) ? hjMedio : hjFrente;
        vT.insert(vT.end(), tronco.begin(), tronco.end());
        vR.insert(vR.end(), ramas.begin(),  ramas.end());
        vH.insert(vH.end(), hojas.begin(),  hojas.end());
    }

    // Cielo con gradiente
    const std::vector<float> vertsCielo = {
        -1.f,-1.00f,0.f, 0.75f,0.87f,0.97f,
         1.f,-1.00f,0.f, 0.75f,0.87f,0.97f,
         1.f, 1.00f,0.f, 0.50f,0.70f,0.92f,
        -1.f,-1.00f,0.f, 0.75f,0.87f,0.97f,
         1.f, 1.00f,0.f, 0.50f,0.70f,0.92f,
        -1.f, 1.00f,0.f, 0.50f,0.70f,0.92f,
    };

    // Suelo (tierra + hierba)
    const std::vector<float> vertsSuelo = {
        -1.f,-1.00f,0.f, 0.30f,0.20f,0.09f,
         1.f,-1.00f,0.f, 0.30f,0.20f,0.09f,
         1.f,-0.86f,0.f, 0.24f,0.38f,0.12f,
        -1.f,-1.00f,0.f, 0.30f,0.20f,0.09f,
         1.f,-0.86f,0.f, 0.24f,0.38f,0.12f,
        -1.f,-0.86f,0.f, 0.24f,0.38f,0.12f,
    };

    // ─── GPU – estáticos ──────────────────────────────────────────────────────
    unsigned int vaoCielo=0,vboCielo=0;
    unsigned int vaoSuelo=0,vboSuelo=0;
    unsigned int vaoTrMF=0,vboTrMF=0, vaoRaMF=0,vboRaMF=0, vaoHjMF=0,vboHjMF=0;
    unsigned int vaoTrF=0,vboTrF=0,   vaoRaF=0,vboRaF=0,   vaoHjF=0,vboHjF=0;
    unsigned int vaoTrM=0,vboTrM=0,   vaoRaM=0,vboRaM=0,   vaoHjM=0,vboHjM=0;
    unsigned int vaoTrFr=0,vboTrFr=0, vaoRaFr=0,vboRaFr=0, vaoHjFr=0,vboHjFr=0;

    subirBuffer(vaoCielo, vboCielo, vertsCielo,  GL_STATIC_DRAW);
    subirBuffer(vaoSuelo, vboSuelo, vertsSuelo,  GL_STATIC_DRAW);
    subirBuffer(vaoTrMF, vboTrMF, trMuyFondo,    GL_STATIC_DRAW);
    subirBuffer(vaoRaMF, vboRaMF, raMuyFondo,    GL_STATIC_DRAW);
    subirBuffer(vaoHjMF, vboHjMF, hjMuyFondo,   GL_STATIC_DRAW);
    subirBuffer(vaoTrF,  vboTrF,  trFondo,       GL_STATIC_DRAW);
    subirBuffer(vaoRaF,  vboRaF,  raFondo,       GL_STATIC_DRAW);
    subirBuffer(vaoHjF,  vboHjF,  hjFondo,       GL_STATIC_DRAW);
    subirBuffer(vaoTrM,  vboTrM,  trMedio,       GL_STATIC_DRAW);
    subirBuffer(vaoRaM,  vboRaM,  raMedio,       GL_STATIC_DRAW);
    subirBuffer(vaoHjM,  vboHjM,  hjMedio,       GL_STATIC_DRAW);
    subirBuffer(vaoTrFr, vboTrFr, trFrente,      GL_STATIC_DRAW);
    subirBuffer(vaoRaFr, vboRaFr, raFrente,      GL_STATIC_DRAW);
    subirBuffer(vaoHjFr, vboHjFr, hjFrente,      GL_STATIC_DRAW);

    // GPU – arbustos dinámicos (se reconstruyen cada frame)
    unsigned int vaoArb=0, vboArb=0;
    {
        std::vector<float> tmp;
        construirArbustos(tmp, arbustos, 0.0f);
        subirBuffer(vaoArb, vboArb, tmp, GL_DYNAMIC_DRAW);
    }

    int nVertsArb = 0;

    // ─── Bucle principal ──────────────────────────────────────────────────────
    while (!glfwWindowShouldClose(window))
    {
        float tiempo = (float)glfwGetTime();
        procesarInput(window);

        // Reconstruir arbustos animados cada frame
        {
            std::vector<float> vertsArb;
            construirArbustos(vertsArb, arbustos, tiempo);
            nVertsArb = (int)(vertsArb.size() / 6);
            glBindBuffer(GL_ARRAY_BUFFER, vboArb);
            glBufferData(GL_ARRAY_BUFFER,
                         (GLsizeiptr)(vertsArb.size()*sizeof(float)),
                         vertsArb.data(), GL_DYNAMIC_DRAW);
        }

        glClearColor(0.72f, 0.85f,0.97f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        shader.use();

        // ── Algoritmo del pintor: dibuja de atrás hacia adelante ─────────────
        // Cada capa se pinta encima de la anterior, simulando profundidad 3D.
        // Dentro de cada capa: hojas primero (fondo) → tronco → ramas encima,
        // así el esqueleto siempre queda visible sobre el follaje.
        // Cielo
        dibujar(vaoCielo, (int)(vertsCielo.size()/6));

        // Muy fondo
        dibujar(vaoHjMF, (int)(hjMuyFondo.size()/6));
        dibujar(vaoTrMF, (int)(trMuyFondo.size()/6));
        dibujar(vaoRaMF, (int)(raMuyFondo.size()/6));

        // Fondo: hojas primero (fondo), luego tronco y ramas encima → skeleton visible
        dibujar(vaoHjF,  (int)(hjFondo.size()/6));
        dibujar(vaoTrF,  (int)(trFondo.size()/6));
        dibujar(vaoRaF,  (int)(raFondo.size()/6));

        // Medio
        dibujar(vaoHjM,  (int)(hjMedio.size()/6));
        dibujar(vaoTrM,  (int)(trMedio.size()/6));
        dibujar(vaoRaM,  (int)(raMedio.size()/6));

        // Frente
        dibujar(vaoHjFr, (int)(hjFrente.size()/6));
        dibujar(vaoTrFr, (int)(trFrente.size()/6));
        dibujar(vaoRaFr, (int)(raFrente.size()/6));

        // Arbustos (primer plano, cubren la base de los troncos)
        dibujar(vaoArb, nVertsArb);

        // Suelo (encima de todo para tapar lo que sobresale por abajo)
        dibujar(vaoSuelo, (int)(vertsSuelo.size()/6));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // ─── Limpieza ─────────────────────────────────────────────────────────────
    auto limpiar = [](unsigned int vao, unsigned int vbo) {
        glDeleteVertexArrays(1, &vao); glDeleteBuffers(1, &vbo);
    };
    limpiar(vaoCielo,vboCielo); limpiar(vaoSuelo,vboSuelo);
    limpiar(vaoTrMF,vboTrMF); limpiar(vaoRaMF,vboRaMF); limpiar(vaoHjMF,vboHjMF);
    limpiar(vaoTrF,vboTrF);   limpiar(vaoRaF,vboRaF);   limpiar(vaoHjF,vboHjF);
    limpiar(vaoTrM,vboTrM);   limpiar(vaoRaM,vboRaM);   limpiar(vaoHjM,vboHjM);
    limpiar(vaoTrFr,vboTrFr); limpiar(vaoRaFr,vboRaFr); limpiar(vaoHjFr,vboHjFr);
    limpiar(vaoArb,vboArb);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
