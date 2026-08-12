# Contrato operativo de resolución

Este documento congela las reglas que conectan `E_C`, `O`, `DO`, las caras
`C4/C5` y la recuperación del diccionario. La implementación normativa está
en Trinity y sus tablas se verifican exhaustivamente en `tests/test_aurora.c`.

## 1. Resolución direccional y tabla completa de E_C

El TriGate conserva la relación:

```text
R = Majority3(A, B, M)
```

La dirección selecciona la única celda transformable:

```text
C=0 -> B (deducción)
C=1 -> R (inferencia)
C=2 -> M (aprendizaje)
```

Para una dirección `C`, se enumeran los valores de la celda transformable que
satisfacen la relación sin modificar sus anclas. La tabla completa queda
definida por el número de valores soportados:

```text
0 soluciones -> E_C=1, incoherencia determinada
1 solución    -> E_C=0, cierre en la dirección C
2/3 soluciones -> E_C=2, ambigüedad
```

Una celda que llega fijada a un valor incompatible produce cero soluciones; el
sistema no la corrige en silencio. Para pedir corrección debe presentarse como
dominio abierto.

### Excepción residual cuando R=2

`R=2` permanece operativo como apertura incluso si la celda transformable
tiene una única solución. En ese caso `E` transporta procedencia:

```text
permutaciones de (2,2,0) -> E=0
permutaciones de (2,2,1) -> E=1
(2,2,2)                  -> E=2
permutaciones de (0,1,2) -> E=2
```

Por ello el estado operacional y el trit `E` no se convierten mediante un cast:
con `R=2`, `E=0/1` es residual y el estado continúa abierto.

## 2. Selección y conservación de O

`O_candidates` conserva todas las posiciones en las que el resultado
mayoritario puede actuar como `ES` sin autorreferencia. `O` selecciona una sola
ruta para el intento actual.

Con fase `p`, la prioridad es:

```text
p, (p+1) mod 3, (p+2) mod 3
```

Se elige la primera posición presente en `O_candidates`. La selección no borra
las restantes: un intento posterior puede recuperarlas mediante `DO`.

Excepciones congeladas:

- `(2,2,2)` conserva las tres rutas y elige `O=2` por prioridad homogénea;
- `(0,1,2)` y `(1,0,2)` no poseen cierre literal por autorreferencia;
- una tripleta imposible puede seguir utilizándose como propósito de búsqueda,
  pero no se registra como cierre observado.

## 3. Recorrido Fibonacci ternario de DO

`DO` dispone exactamente de 27 estados. El intento `t`, con `0 <= t < 27`,
utiliza:

```text
n_t = 7t mod 27
```

El número `n_t` se expresa con tres trits. El paso `7` es `F(9) mod 27`, pues
`F(9)=34`, y es coprimo con 27; por tanto genera una permutación completa sin
repeticiones.

Los primeros estados son:

```text
000, 021, 112, 210, 001, ...
```

Un mapa de 27 bits registra los estados consumidos. Tras el intento 27 no queda
otro estado admisible y la búsqueda se declara agotada.

### DO y orientación de U

`DO_t` es el estado consumido por el recorrido; `O_U` es la orientación de la
salida U durante ese intento. Al comenzar un intento explícito:

```text
O_U <- DO_t
```

Si U* se materializa como emergencia o carry, el tensor conserva `O_U` junto
con su procedencia. Un carry usado como A en la ventana siguiente transmite
esa orientación. Una U sin historia previa parte de `(0,0,0)`. De este modo el
cursor puede continuar avanzando sin borrar la orientación del resultado que
produjo cada intento.

## 4. Simultaneidad de C4 y C5

La regla de resolución es:

```text
C5 cerrada                         -> inferir/aceptar R
C5 incoherente                     -> aprender M alternativo
C4 cerrada con C5 abierta          -> inferir R
C4 incoherente con C5 abierta      -> deducir B
C4 abierta y C5 abierta, fase 0    -> deducir B
C4 abierta y C5 abierta, fase 1    -> inferir R
C4 abierta y C5 abierta, fase 2    -> aprender M
DO agotado                         -> desistir sin inventar cierre
```

La fase utilizada en el empate es el trit menos significativo del estado `DO`
actual. C6 conserva el recorrido y determina cuándo ya no queda gasto.

## 5. Reejecución de candidatas del diccionario

La consulta geométrica devuelve todas las candidatas compatibles ordenadas por
uso y recencia. La candidata actúa como conocimiento, no como tercer token:

```text
(A, B, U=abierto; K_candidata, DO_t) -> (DS,DE,DO)
```

Cada intento se evalúa sin modificar la arena. Las candidatas se recorren
circularmente mientras `DO` avanza por sus 27 estados.

- Si una reejecución alcanza cierre, se materializa una sola ventana, se
  conserva la candidata en la procedencia, se fija `O_U=DO_t` y se actualiza
  su prioridad.
- Si permanece abierta o resulta incoherente, no se escribe ningún tensor.
- Si ninguna cierra, el resultado solicita calcular una alternativa nueva.
- Si además se consumen los 27 estados, la búsqueda queda marcada como
  agotada.

De esta forma, encontrar una geometría compatible nunca equivale a aceptarla:
la aceptación depende siempre de una reejecución completa de la ventana.
