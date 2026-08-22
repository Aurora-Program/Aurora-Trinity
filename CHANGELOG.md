# Changelog

## Sin publicar

- Mueve `aurora_circular_census` dentro de `BUILD_TESTING`, de modo que la
  biblioteca se configure sin necesitar las fuentes de ejemplos.
- Fija `BUILD_TESTING=OFF` en la receta Conan; el recipe solo necesita
  `CMakeLists.txt`, `cmake/`, `include/`, `src/` y `genesis/` para construir
  e instalar las bibliotecas.
- Elimina los borradores residuales `README-new.md` y `Licenses-new.md`;
  `README.md` y `LICENSE` son los canónicos.
- Documenta explícitamente el caso residual `(2,2,2)` en `trigate.c` y el
  estado todavía inerte de los umbrales del armonizador en `executive.h`.

## 0.1.0 - Pre-release

- Publica el núcleo Trinity como biblioteca C11 con integración CMake.
- Incluye propagación de TriGates, ordenación, redes, ventanas, secuencias,
  diccionario, traducción y persistencia transaccional.
- Incluye Project Genesis como módulo experimental separado.
- Añade pruebas exhaustivas del núcleo y pruebas deterministas de Genesis.
- Incluye una receta experimental para empaquetado con Conan 2.
- Añade CI para Windows y Linux, incluida la prueba de consumo desde una
  instalación CMake.
- Documenta la procedencia y el checksum de la release `v0.1.0`.
- El snapshot de esta versión es binario y depende de la misma ABI de
  compilación.

Esta versión no representa todavía una API estable ni un formato de snapshot
portable.
