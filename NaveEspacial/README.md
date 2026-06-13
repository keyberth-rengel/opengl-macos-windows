# NaveEspacial

Proyecto del ejercicio de nave espacial basado en el proyecto `Shader`.

## Que hace

- Dibuja una nave simple formada por dos triangulos
- La nave se mueve con `W`, `A`, `S`, `D`
- Tambien acepta las flechas del teclado
- La nave no puede salir del contenedor de la ventana
- Caen tres asteroides desde arriba
- Los asteroides vuelven a aparecer arriba cuando salen por abajo
- `Shift` activa el modo turbo
- El turbo acelera la nave y tambien la caida de los asteroides
- El turbo cambia el color de la nave

## Archivos importantes

- `src/main.cpp`: nave, movimiento, limites, turbo y actualizacion de asteroides
- `res/Shader/vertexShader.glsl`: aplica desplazamiento y escala
- `res/Shader/fragmentShader.glsl`: cambia color de la nave en turbo y deja color fijo para asteroides
- `src/Shader.h` y `src/Shader.cpp`: clase minima para cargar shaders y enviar uniforms

## Ejecutar en Mac

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix)"
cmake --build build
./build/NaveEspacial
```
