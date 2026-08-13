# Modelo Aurora

Estado: pre-release experimental (`0.1.0`). La API y el comportamiento pueden
cambiar antes de una versión estable.

Este repositorio contiene la documentación conceptual de Aurora, la biblioteca
de ejecución Trinity y el programa experimental Project Genesis.

## Estado actual

La biblioteca `Trinity::trinity` implementa:

- los siete dominios no vacíos de `{0,1,2}`;
- `Majority3` sobre valores;
- la extensión de `Majority3` sobre dominios;
- propagación bidireccional local de un TriGate hasta punto fijo;
- coherencia global exacta como conjunción simultánea de restricciones, no como
  agregación de decisiones locales;
- los estados de relación `E=0` (coherencia/cierre), `E=1` (separación
  determinada o incoherencia) y `E=2` (ambigüedad/apertura).
- paquete observable `(R,E_C,O)` para la ordenación, resolución direccional
  `C=0/1/2` y conservación explícita de empates en `O`.
- ordenación de tripletas en las dimensiones `ES`, `FN` y `FO`, conservando
  como dominio todas las orientaciones posibles.
- redes de TriGates con celdas compartidas y propagación circular hasta punto
  fijo;
- operación mínima sobre tres tripletas, incluyendo los tres pares del
  destilador razonador y la producción de `DS`, `DE` y `DO`.
- inyección opcional de conocimiento `DS`, que participa en el mismo punto fijo
  y puede retropropagar restricciones o detectar contradicciones.
- tensor fractal ternario con procedencia explícita, profundidad y número de
  hojas;
- semilla operativa explícita: tres tripletas inferiores `DS/DE/DO` y un `DS`
  superior sintetizado operando esas tres tripletas con el mismo núcleo.
- ventana atómica sobre tres SO superiores, con `E_W` de contradicción
  dominante, orientación propia `O_U` y acciones explícitas de emergencia,
  carry o deslizamiento.
- recorrido completo de un nivel con cursor, carry encadenado, recuperación por
  deslizamiento y devolución explícita del final pendiente.
- transcender con caras invariantes C4/C5/C6, armonización configurable y
  semilla ejecutiva `(δS,δE,δO) → (R,E,O)`.
- diccionario emergente con candidatas nacidas de entradas, verificación de uso
  por procedencia y cristalización ejecutiva.
- índice asociativo ternario por síntesis superior, consulta asimétrica
  (`2` abre las tres ramas), descenso AND por los tres destinos, conservación
  de alternativas y prioridad por cierres/recencia.
- reejecución automática de candidatas como conocimiento sobre `U` abierto;
  solamente una candidata con `DE=000` materializa y asciende una ventana.
- recorrido `DO` exhaustivo de 27 estados con paso Fibonacci ternario, y regla
  explícita para resolver la apertura simultánea de C4/C5.
- traducción por coincidencia más larga, sustitución tensorial, validación con
  el mismo recorrido jerárquico y backoff a unidades menores.
- arranque desde conocimiento vacío V2, desalojo LRU y ciclo integral de
  aprendizaje que reduce una frase conocida de una ventana a cero.
- snapshot atómico conjunto de diccionario y arena tensorial, con versión,
  checksum, procedencia, estados, usos, recencia y reloj lógico.

El nivel de abstracción y la profundidad de procedencia se almacenan por
separado. Un carry aumenta su procedencia pero permanece en el mismo nivel que
los tensores siguientes; solamente una emergencia `E_W=0` asciende de nivel.
El recorrido jerárquico no redefine la operación: entrega las emergencias de un
nivel a la misma función `run_level` hasta obtener una raíz o un bosque final.

El armonizador recibe tres umbrales explícitos (`near_ds`, `near_de`,
`near_do`). La configuración inicial usa distancia 1, pero no se presenta como
calibración aprendida. La semilla ejecutiva ya puede evaluar una terna explícita
de SO de entrada, conocimiento y salida. Todavía no gobierna automáticamente la
secuencia porque el runtime aún no dispone de una memoria que asigne el papel
de conocimiento; esa conexión no se infiere de tres tokens consecutivos.

## Persistencia

El estado operativo se guarda y recupera conjuntamente:

```c
aurora_snapshot_save("aurora.snapshot", &dictionary, &tensor);
aurora_snapshot_load("aurora.snapshot", &dictionary, &tensor);
```

El guardado escribe primero un archivo temporal y lo sustituye atómicamente al
terminar. El formato incluye versión y checksum; una carga truncada, alterada o
incompatible se rechaza sin modificar el estado residente. La versión 3 guarda
estructuras binarias y comprueba sus tamaños, por lo que está destinada al
mismo ABI de compilación. Una futura versión portable deberá serializar los
campos individualmente.

La distinción fundamental de V2 es:

```text
{2}       valor 2 cerrado
{0,1,2}   celda sin información suficiente
```

Una hoja tensorial conserva su tripleta como `DS` superior. Como todavía no
procede de una emergencia, sus canales `DE` y `DO` permanecen en `{0,1,2}`; no
se fabrican estados de cierre ni orientaciones inexistentes. Cada composición
relaciona los `DS` superiores de las tres SO participantes. El resultado de esa
relación forma los canales inferiores `DS/DE/DO` de la nueva SO, y estos tres
canales se operan de nuevo para obtener su `DS` superior.

El archivo `Aurora-semilla.c` se conserva como prototipo experimental original.
Todavía no forma parte de la biblioteca.

La especificación ejecutable de la biblioteca está en `docs/TRINITY.md`. El trabajo sobre
el origen de los primeros tensores semánticos vive separadamente en
`genesis/README.md`. El contrato específico de `O_U` está en
`docs/U_WINDOW_ORIENTATION.md`.

## Compilar y verificar

Requisitos: compilador de C11 y CMake 3.16 o posterior.

```sh
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

Las pruebas recorren las 27 entradas de `Majority3`, las 343 combinaciones de
tres dominios y los 2.401 estados posibles de los cuatro puertos de un TriGate.
También comprueban propagación inversa, contradicción, monotonía e idempotencia.
Para la ordenación recorren las 27 tripletas concretas y verifican el censo
formal: 16 orientaciones únicas, 9 ambiguas y 2 imposibles.
La operación mínima se ejecuta sobre las 2.300 selecciones de tres tripletas
válidas y distintas. Cada salida comprueba conjuntamente `DS`, `DE` y `DO`,
incluido el transporte residual excepcional cuando `R=2`.

## API

La API pública está en:

- `include/aurora/domain.h`
- `include/aurora/trigate.h`
- `include/aurora/ordering.h`
- `include/aurora/network.h`
- `include/aurora/operation.h`
- `include/aurora/so.h`
- `include/aurora/tensor.h`
- `include/aurora/window.h`
- `include/aurora/sequence.h`
- `include/aurora/executive.h`
- `include/aurora/dictionary.h`
- `include/aurora/translation.h`
- `include/aurora/bootstrap.h`
- `include/aurora/persistence.h`

El índice tensorial asociativo y la lexicalización por tokens permanecen como
dos entradas compatibles al mismo conocimiento. La recuperación geométrica ya
reejecuta cada candidata antes de aceptarla. La red distribuida se incorporará
sobre este contrato de verificación.

## Documentación existente

- `overview.md`: motivación y visión conceptual.
- `Especificaciones Modelo aurora.md`: especificación histórica de Aurora V1.
- `analisis.md`: decisiones y resultados experimentales que motivan V2.
- `diccionariocompresion.md`: memoria, compresión y compartición distribuida.

Para V2, `docs/TRINITY.md`, los contratos específicos de `docs/` y las pruebas
constituyen la referencia ejecutable del núcleo. No existe todavía una
especificación V2 consolidada en un único documento.


