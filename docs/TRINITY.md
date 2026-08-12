# Trinity 0.1

Trinity es la biblioteca de ejecución del Modelo Aurora. Su responsabilidad es
operar relaciones; no asigna significado inicial a los tokens.

## Núcleo congelado en 0.1

- dominios no vacíos de `{0,1,2}`;
- TriGate reversible y redes circulares hasta punto fijo;
- ordenación `ES/FN/FO`;
- operación mínima y destilador razonador;
- semillas operativas `DS/DE/DO`;
- tensores fractales con procedencia verificable;
- ventanas, carry, secuencias y recurrencia entre niveles;
- transcender `C4/C5/C6` y semilla ejecutiva `R/E/O`;
- diccionario emergente, traducción y backoff;
- arranque, LRU y persistencia transaccional.

Trinity no contiene embeddings, corpus, tokenizadores estadísticos ni pesos
entrenados. Es el sustrato común contra el que deben evaluarse todos los
experimentos de Project Genesis.

## Uso

```c
#include <trinity/trinity.h>
```

Con CMake, el target instalado es:

```cmake
target_link_libraries(mi_programa PRIVATE Trinity::trinity)
```

Durante la transición sigue disponible el alias de compilación `aurora_core`.

El módulo experimental Genesis se enlaza separadamente:

```cmake
target_link_libraries(mi_experimento PRIVATE Trinity::genesis)
```

## Compatibilidad

La API pública conserva el prefijo `aurora_`: Aurora es el modelo y Trinity su
motor. Los snapshots 0.1 son binarios y requieren la misma versión y ABI.
