# Publicar en ConanCenter

La receta local está en `conanfile.py` y usa Conan 2. El paquete se llama
`trinity` y la versión inicial es `0.1.0`.

## Verificación local

Instala Conan 2 y ejecuta desde la raíz del repositorio:

```sh
conan profile detect --force
conan create . --version=0.1.0
```

La receta configura, compila e instala el proyecto mediante CMake. El paquete
expone las bibliotecas `genesis` y `trinity` y sus headers públicos.

## Pull Request a ConanCenter

1. Publica primero una release de GitHub con la tag `v0.1.0`.
2. Crea un fork de `https://github.com/conan-io/conan-center-index`.
3. Añade la receta bajo `recipes/trinity/all/` siguiendo la estructura vigente
   de ConanCenter.
4. Ejecuta las validaciones de ConanCenter localmente.
5. Abre un Pull Request contra `conan-io/conan-center-index`.

La receta de este repositorio sirve como punto de partida. ConanCenter puede
pedir ajustes de estilo, fuentes, licencias o separación del módulo
experimental Genesis antes de aceptar el paquete.