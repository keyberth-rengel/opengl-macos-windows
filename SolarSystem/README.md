# SolarSystem

Proyecto OpenGL 3D con texturas, compatible con macOS y Windows usando la misma base de `main.cpp`.

## macOS

Comandos exactos:

```bash
cd /Users/keyberthrengel/Developer/OpenGL/SolarSystem
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build
./build/SolarSystem
```

Si quieres recompilar desde cero:

```bash
cd /Users/keyberthrengel/Developer/OpenGL/SolarSystem
rm -rf build
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build
./build/SolarSystem
```

## Windows

Abrir la carpeta del proyecto en Visual Studio y compilar en `x64`.

Dependencias esperadas:

- `GLFW`
- `GLEW`
- `OpenGL`

La base actual queda separada por plataforma:

- En Windows usa `GLEW`
- En macOS usa `GLAD`

## Notas

- El ejecutable final ahora se llama `SolarSystem`.
- `stb_image.cpp` ya est incluido en CMake para que las texturas compilen correctamente.
- En `main.cpp` se corrigi la carga de OpenGL para macOS y se dej coherente el uso de texturas con el fragment shader.
