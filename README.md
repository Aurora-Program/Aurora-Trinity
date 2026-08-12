# Modelo Aurora

Aurora contiene la biblioteca de ejecución Trinity y el programa experimental
Project Genesis.

## Estado

Versión `0.1.0`, pre-release experimental. La API, el formato de snapshot y
los contratos operativos pueden cambiar antes de una versión estable.

El núcleo Trinity está implementado en C11. Project Genesis es experimental y
se enlaza como un módulo separado.

## Compilar y verificar

Requisitos: compilador C11 y CMake 3.16 o posterior.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## Documentación

- [Descripción detallada y estado](README-new.md)
- [Contrato de Trinity](docs/TRINITY.md)
- [Orientación de ventanas](docs/U_WINDOW_ORIENTATION.md)
- [Contratos de resolución](docs/RESOLUTION_CONTRACT.md)
- [Project Genesis](genesis/README.md)
- [Historial de Aurora V1](Especificaciones%20Modelo%20aurora.md)
- [Notas de la release 0.1.0](docs/RELEASE.md)

![CI](https://github.com/Aurora-Program/Aurora-Trinity/actions/workflows/ci.yml/badge.svg)

## Compatibilidad

La API pública usa el prefijo `aurora_` y se incluye mediante
`<trinity/trinity.h>`. Los snapshots de la versión 0.1 son binarios y solo
son compatibles con la misma versión y ABI de compilación; no deben tratarse
como un formato portable entre compiladores o arquitecturas.

## Conan

La receta experimental para Conan 2 está en `conanfile.py`. Las instrucciones
para probarla y proponerla a ConanCenter están en
`docs/PUBLISHING_CONAN.md`.

## Licencia

Consulta [LICENSE](LICENSE) para el alcance de las licencias de código y
documentación.