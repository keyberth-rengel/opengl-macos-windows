# LSystemForest

Proyecto separado para la evaluación T1. Genera un bosque en 2D usando un motor L-System real.

Cumple con lo esencial del enunciado:

- motor L-System separado del render
- escena 2D
- al menos 8 árboles
- árboles distintos entre sí
- hojas frondosas simples
- arbustos en la base
- arbustos moviéndose con un solo ritmo

## Mac

```bash
cd /Users/keyberthrengel/Developer/OpenGL/LSystemForest
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build
./build/LSystemForest
```

## Windows

Compila con Visual Studio o CMake usando la misma idea del proyecto `Shader`:

- `GLEW` en Windows
- `GLAD` en macOS
- arquitectura consistente (`x64` con librerías `x64`)

Con CMake:

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```
