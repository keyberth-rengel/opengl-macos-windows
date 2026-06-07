# Shader

Proyecto minimo con `GLFW` y una clase `Shader` para cargar:

- `res/Shader/vertexShader.glsl`
- `res/Shader/fragmentShader.glsl`

El punto de entrada es `src/main.cpp`.

## Mac

```bash
cd /Users/keyberthrengel/Developer/OpenGL/Shader
cmake -S . -B build-clean -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build-clean
./build-clean/Shader
```

## Windows

Abre el proyecto en Visual Studio y compila con tus dependencias de `GLEW` y `GLFW`.

El `main.cpp` ya usa:

- `GLEW` en Windows
- `GLAD` en macOS

Si copias este proyecto a Windows, asegúrate de que `Shader.cpp` también esté agregado al proyecto.
