# OpenGLProject (macOS)

Proyecto base de OpenGL en C++ para macOS con GLFW, GLAD y CMake.

## 1) Dependencias

```bash
xcode-select --install
brew install glfw cmake
```

## 2) Descargar GLAD

Genera GLAD con:

- Language: `C/C++`
- Specification: `OpenGL`
- API: `gl 4.1`
- Profile: `Core`

Copia los archivos a estas rutas:

- `glad/include/glad/glad.h` -> `include/glad/glad.h`
- `glad/include/KHR/khrplatform.h` -> `include/KHR/khrplatform.h`
- `glad/src/glad.c` -> `libs/glad.c`

## 3) Compilar

```bash
cmake -S . -B build
cmake --build build
./build/OpenGLProject
```

Si CMake no encuentra GLFW:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build
./build/OpenGLProject
```
