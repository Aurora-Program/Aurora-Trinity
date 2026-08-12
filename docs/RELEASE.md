# Release 0.1.0

La release `v0.1.0` es una pre-release experimental de Trinity y Project
Genesis.

## Procedencia

- Repositorio fuente: https://github.com/Aurora-Program/Aurora-Trinity
- Tag: `v0.1.0`
- ConanCenter: PR [#30779](https://github.com/conan-io/conan-center-index/pull/30779)
- Hugging Face: https://huggingface.co/AuroraProgram/aurora-trinity

El archivo fuente de la tag se publica como:

```text
https://github.com/Aurora-Program/Aurora-Trinity/archive/refs/tags/v0.1.0.tar.gz
```

SHA256 del archivo fuente:

```text
ebbcd34dc8e749ea493e51f413b69fb0b096f4d641fd9abc0a069be488ce74f5
```

El artefacto `genesis_dictionary.bin` publicado en Hugging Face corresponde a
la misma revisión experimental y no representa un modelo neuronal entrenado.

## Validación

La fuente de la release se ha verificado con una configuración CMake limpia y
las tres pruebas CTest del proyecto. La rama actual añade además un consumidor
externo que usa los targets instalados `Trinity::trinity`.