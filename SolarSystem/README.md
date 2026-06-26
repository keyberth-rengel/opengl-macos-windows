# SolarSystem

Proyecto de OpenGL en C++ que renderiza un sistema solar 3D con texturas y camara interactiva.

## Incluye

- Sol central texturizado
- 8 planetas orbitando alrededor del Sol
- Luna orbitando la Tierra
- Anillo de Saturno con textura propia
- Fondo estelar
- Rotacion propia de cada cuerpo
- Orbitas visibles que se pueden activar o desactivar
- Camara orbital con foco sobre planetas

## Estructura del proyecto

- `src/main.cpp`: organiza la escena, crea mallas, carga texturas, controla la camara y renderiza el sistema.
- `src/Shader.h` y `src/Shader.cpp`: clase auxiliar para cargar, compilar y usar shaders.
- `res/Shader/`: shaders de vertices y fragmentos.
- `res/Texture/`: texturas del Sol, planetas, Luna, anillo y fondo.
- `include/` y `libs/`: dependencias usadas para compilar en macOS.
- `src/vendor/`: librerias de apoyo del proyecto.

## Requisitos

### macOS

- Xcode Command Line Tools
- Homebrew
- `cmake`
- `glfw`

### Windows

- Visual Studio
- GLFW configurado en el proyecto
- GLEW configurado en el proyecto

## Ejecutar en macOS

Usar estos comandos exactos:

```bash
cd /Users/keyberthrengel/Developer/OpenGL/SolarSystem
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build
./build/SolarSystem
```

## Ejecutar en Windows

Si el proyecto ya tiene GLFW, GLEW y las texturas enlazadas en Visual Studio:

1. Abrir la solucion o proyecto.
2. Seleccionar `x64`.
3. Compilar con `Build Solution`.
4. Ejecutar con `Local Windows Debugger` o `Ctrl + F5`.

## Controles

### Camara

- `A` o ``: girar la camara a la izquierda
- `D` o ``: girar la camara a la derecha
- `W` o ``: subir la inclinacion
- `S` o ``: bajar la inclinacion
- `Q`: acercar la camara
- `E`: alejar la camara

### Simulacion

- `Space`: pausar o reanudar el movimiento
- `-`: bajar la velocidad global
- `+`: subir la velocidad global
- `O`: mostrar u ocultar orbitas
- `R`: restablecer camara y simulacion
- `Esc`: cerrar la ventana

### Foco por planeta

- `1`: enfocar y acercar a Mercurio
- `2`: enfocar y acercar a Venus
- `3`: enfocar y acercar a la Tierra
- `4`: enfocar y acercar a Marte
- `5`: enfocar y acercar a Jupiter
- `6`: enfocar y acercar a Saturno
- `7`: enfocar y acercar a Urano
- `8`: enfocar y acercar a Neptuno
- `0`: volver a la vista general

## Que hace cada parte visual

- El Sol queda en el origen y actua como referencia central de toda la escena.
- Cada planeta usa una textura distinta y gira alrededor del Sol con su propia velocidad.
- Cada planeta tambien rota sobre su propio eje para que la escena se vea viva.
- La Luna depende de la Tierra y gira alrededor de ella.
- Saturno dibuja el planeta y luego un anillo aparte con otra textura.
- El fondo usa una esfera grande con textura de estrellas para cerrar la escena.
- Las orbitas se dibujan como lineas para ayudar a leer mejor el movimiento.

## Donde cambiar cosas rapidamente

### Si quieres cambiar tamanos

Editar `radius` dentro del arreglo `bodies` en `src/main.cpp`.

### Si quieres cambiar distancias orbitales

Editar `orbitRadius` en el arreglo `bodies` de `src/main.cpp`.

### Si quieres cambiar velocidades

Editar `orbitSpeed` y `selfRotationSpeed` en `src/main.cpp`.

### Si quieres cambiar la camara inicial

Editar `CameraState camera` y `defaultCamera` en `src/main.cpp`.

### Si quieres cambiar controles

Editar el bloque de input dentro del `while` principal en `src/main.cpp`.

## Compatibilidad

- En macOS el proyecto usa `GLAD`.
- En Windows el proyecto usa `GLEW`.
- La seleccion entre ambos se hace con `#ifdef _WIN32` dentro de `src/main.cpp`.
