# Project Genesis

## Pregunta central

**¿Cómo crea Aurora su primer significado?**

El marco complementario **Butterfly / Metamorphosis** interpreta Genesis como
una transición progresiva desde representaciones neuronales privadas hacia un
espacio fractal público. El LLM puede actuar como fuente o profesor durante la
transición, pero Trinity continúa siendo el criterio final de operación,
procedencia y cierre.

Trinity ya puede operar, verificar, compartir y persistir tensores. Genesis no
debe modificar ese operador para favorecer una hipótesis de origen. Su tarea es
producir los primeros tensores semánticos públicos para tokens simples y medir
qué organización aparece al procesarlos con la misma biblioteca congelada.

## Definición operacional de significado

En la primera etapa, un tensor posee significado útil si, comparado con los
controles, mejora de forma reproducible:

- el cierre de relaciones contextuales;
- la discriminación entre contextos incompatibles;
- la transferencia a secuencias no observadas;
- la reconstrucción desde su procedencia;
- el coste en ventanas después del aprendizaje.

Ser diferente de otros tensores no es suficiente.

## Tres rutas experimentales

### G-R — Origen pseudoaleatorio

Asignar tensores mediante una semilla pública y determinista. Es el control
fundamental: mide cuánta estructura puede producir Trinity sin herencia
semántica. No se utilizará aleatoriedad irreproducible.

### G-E — Destilación de embeddings

Aprender una transformación desde embeddings existentes hacia tensores Aurora.
La transformación puede heredar geometría útil, pero el resultado solo se
acepta si cierra y se verifica dentro de Trinity. El embedding es profesor, no
parte del formato público final.

El primer componente de esta ruta recibe el nombre provisional `ffe_encoder`.
Debe producir exactamente el mismo contrato tensorial público que G-R y G-3;
Genesis no tendrá un formato distinto para cada origen.

### G-3 — Base semántica mínima

Partir de tres tensores fundamentales públicos y generar el resto mediante sus
interacciones. Es la hipótesis más fuerte y la que exige mayor cuidado para no
introducir manualmente la semántica que pretende explicar.

## Protocolo común

Las tres rutas usarán:

1. el mismo vocabulario y corpus;
2. las mismas particiones de entrenamiento, validación y novedad;
3. semillas y configuraciones publicadas;
4. Trinity 0.1 sin modificaciones;
5. snapshots y resultados reproducibles;
6. controles negativos y ablaciones.

Métricas iniciales:

- colisiones tensoriales;
- tasa de cierre, apertura y contradicción;
- ventanas por secuencia antes y después de aprender;
- estabilidad ante repetición;
- sensibilidad al contexto;
- exactitud de reconstrucción;
- igualdad entre nodos al re-ejecutar;
- tamaño del diccionario y tasa de backoff.

Las comparaciones de memoria deben distinguir al menos tres magnitudes:

- representación desplegada;
- representación canónica serializada;
- snapshot completo con procedencia, `DE`, `DO` y diccionario.

No se comparará un tensor aislado con todos los parámetros de un LLM como si
fueran unidades equivalentes.

## Orden de ejecución recomendado

1. Implementar G-R como baseline barato y necesario.
2. Ejecutar G-E contra exactamente el mismo protocolo.
3. Evaluar G-3 cuando exista una línea base capaz de detectar si realmente
   aporta estructura y no solo una codificación distinta.

El primer entregable de Genesis será un generador G-R determinista y un informe
de métricas, no una afirmación de semántica conseguida.

## Primer baseline G-R implementado

El módulo experimental ya implementa el payload de 117 bits y un generador
pseudoaleatorio determinista:

```c
genesis_tensor117 tensor;
genesis_gr_generate(public_seed, token_id, &tensor);
```

La estructura física es:

```text
bytes 0..14
└── 39 máscaras consecutivas de 3 bits
    ├── raíz:             3 dominios
    ├── nivel intermedio: 9 dominios
    └── hojas:           27 dominios
```

G-R genera primero nueve tripletas hoja reproducibles. Trinity sintetiza tres
nodos intermedios y una raíz; solo entonces se serializa el árbol. Al importar,
los niveles superiores se vuelven a derivar y deben coincidir exactamente con
el payload. Por tanto, un bloque de 117 bits aleatorio pero incoherente no se
acepta como tensor Genesis.

Baseline inicial con seed pública `42`:

- 256 tokens generados;
- cero colisiones observadas;
- igualdad exacta al regenerar el mismo token;
- round-trip payload→Trinity→payload exacto.

Este resultado valida formato y reproducibilidad, no significado. La siguiente
medición debe usar contextos y cierre para determinar si aparece organización
por encima del control pseudoaleatorio.

## Primer benchmark contextual

Se usaron doce tokens divididos externamente en tres categorías, seis frases de
entrenamiento y seis combinaciones internas no vistas. G-R se comparó con un
control que conserva exactamente los mismos tensores pero baraja la asignación
token↔tensor.

| Métrica | G-R | Control barajado |
|---|---:|---:|
| Ventanas en primera lectura | 6 | 6 |
| Ventanas al repetir lo visto | 0 | 0 |
| Ventanas en combinaciones nuevas | 6 | 6 |
| Cierres | 0 | 0 |
| Aperturas | 12 | 12 |
| Contradicciones | 0 | 0 |
| Colisiones de raíz en 6 heldout | 5 | 5 |
| Colisiones de SO completa en 6 heldout | 5 | 5 |

El resultado es nulo en el sentido correcto: G-R permite lexicalizar y recordar
frases exactas, pero no transfiere todavía la estructura categorial y no supera
al control. La colisión de raíces muestra además que el `DS` superior es
demasiado grueso como discriminante aislado. En este ensayo `DE/DO` tampoco
recuperan la diferencia: las seis combinaciones heldout colapsan a una misma SO.
Antes de atribuir semántica a cualquier encoder, Genesis deberá exigir que
supere explícitamente este colapso frente al control.

## Aprendizaje por conversación educadora

El profesor desarrolla conocimiento, pero no interpreta la representación
privada del estudiante. Puede enseñar experiencias, hacer preguntas y corregir
respuestas; no asigna significado a celdas, caras o distancias internas. El
contrato completo se formaliza en `PEDAGOGY.md`.

G-R inicializa identidades reproducibles; no enseña significado. El mecanismo
de aprendizaje de Genesis es una conversación determinista con un educador:

```text
experiencia
→ pregunta abierta
→ corrección
→ comparación con la historia
→ discriminante sin contraejemplos
→ regla usada en una salida
→ cristalización en el diccionario
```

El educador proporciona hechos y correcciones, no tensores finales ni reglas.
Genesis solo cristaliza un discriminante cuando aparece en al menos dos sujetos
con la misma salida y no existe un sujeto conocido con ese discriminante y una
salida diferente.

Primera lección:

- gato y perro viven con humanos y duermen en casa;
- vaca vive en granja y duerme en establo;
- hámster solo recibe el hecho `vive_con humanos`;
- transferencia obtenida: `hámster duerme_en casa`;
- murciélago y águila vuelan pero duermen en lugares distintos, por lo que
  `vuela` se rechaza como discriminante;
- tras dos ejemplos compatibles con `tiene plumas`, se transfiere
  `halcón duerme_en nido`.

Hechos, correcciones, tokens y reglas viven en el mismo diccionario Trinity con
espacios de nombres diferentes. El snapshot conjunto conserva toda la historia
educativa; después de reiniciar, ambas respuestas transferidas permanecen.

### Discriminantes compuestos y excepciones

Una propiedad aislada puede ser insuficiente. Genesis combina de forma
canónica hasta tres pares `(relación, objeto)` nacidos de los hechos del sujeto.
Cada conjunción se somete al mismo criterio educativo: al menos dos casos de
soporte y ningún contraejemplo conocido dentro de esa conjunción. No existe un
segundo motor para las excepciones.

Cuando varias reglas son aplicables, prevalece la que contiene más condiciones.
Dos resultados distintos con igual especificidad producen una respuesta
ambigua; un hecho literal prevalece siempre sobre cualquier regla.

La segunda lección conserva `tiene plumas -> duerme en nido`. La propiedad
`negro` aislada tiene un contraejemplo y no puede cristalizar. Dos correcciones
compatibles permiten, sin embargo, cristalizar la relación más precisa:

```text
tiene plumas AND tiene negro -> duerme en torre
```

Un halcón con plumas obtiene primero `nido`; al aprender que también es negro,
obtiene `torre`. La regla general no se borra: queda subordinada por
especificidad. La regla compuesta y su resultado sobreviven al snapshot.

### Rectificación con memoria histórica

`genesis_education_correct` distingue una corrección de la simple incorporación
de otro hecho. Si ya existe una salida diferente para el mismo sujeto y
relación:

1. cristaliza una revisión `anterior -> nueva`;
2. retira el hecho anterior de la evidencia activa;
3. incorpora el hecho corregido;
4. vuelve a evaluar las reglas cuando intentan producir una salida.

La retirada no elimina la entrada: cambia su estado y el snapshot conserva
tanto el hecho reemplazado como la revisión y el hecho vigente. Una regla
cristalizada tampoco se acepta ciegamente. Debe seguir teniendo dos soportes
activos y ningún contraejemplo; si una corrección destruye ese fundamento, la
regla permanece como historia pero deja de operar.

La prueba educativa rectifica `zorro duerme en cueva` a `zorro duerme en casa`
y recupera únicamente `casa` después de reiniciar. También rectifica uno de los
dos soportes de una regla general y comprueba que un sujeto nuevo vuelve a
estado desconocido.

### Currículo acumulativo en disco

La primera conversación reutilizable vive en
`genesis_curriculum_foundation`. Puede aplicarse sobre una educación vacía o
sobre un snapshot cargado. Repetirla no añade hechos, reglas ni tensores: la
segunda ejecución conserva exactamente los mismos contadores.

El ejecutable `genesis_educate_baby` carga primero el conocimiento existente,
aplica el currículo y guarda atómicamente el resultado. El estado canónico del
primer estudiante se conserva en:

```text
genesis/knowledge/genesis_dictionary.bin
```

Su correspondencia pública `id -> palabra` está en `lexicon.tsv`. El binario no
es un artefacto aislado: ambos archivos forman juntos la memoria operativa y su
lectura humana.

La segunda conversación amplía el conocimiento sin modificar Trinity ni el
inductor. León y tigre reciben la experiencia `come carne` y la corrección
`es carnívoro`; el lobo sólo recibe la experiencia. Aurora transfiere:

```text
lobo come carne -> lobo es carnívoro
```

La explicación señala una condición y dos soportes educativos. El snapshot
crece de 38 a 51 entradas y de 221 a 312 nodos. Una nueva ejecución conserva
exactamente 51/312: recordar incrementa el uso de la regla, pero no duplica su
contenido.

La tercera conversación introduce respiración acuática sin alterar el motor:

```text
pez, tiburón       + vive en agua -> respira por branquias
delfín, ballena    + vive en agua + es mamífero -> respira por pulmones
salmón (no visto)  + vive en agua -> branquias
foca (no vista)    + vive en agua + es mamífero -> pulmones
```

Las dos transferencias se resuelven con soporte dos. La del salmón usa la regla
general de una condición; la de la foca usa la excepción contextual de dos
condiciones. El snapshot pasa a 79 entradas y 468 nodos y permanece idempotente
en la siguiente sesión. Con el límite actual de 512 nodos quedan 44 libres; las
próximas pruebas deberían reutilizar el vocabulario o abordar explícitamente la
segmentación/versionado de memoria, no aumentar silenciosamente el límite.

La cuarta conversación conecta dominios sin crear nuevas entidades. Tiburón,
delfín, águila y foca reciben `come carne` y reutilizan la relación aprendida de
los dos ejemplos león/tigre. El primer intento formuló el resultado como
`ES carnívoro` y fue rechazado correctamente por el currículo: `ES mamífero`
aparecía como si fuera una salida incompatible, porque las relaciones objetivo
son funcionales en esta etapa.

La educación se reformuló con un único token relacional nuevo, `DIETA`, sin
borrar `ES mamífero`:

```text
delfín ES mamífero
delfín DIETA carnívoro
tiburón DIETA carnívoro
tiburón ES ? -> desconocido
```

Esto no es sólo una corrección de nombres: evita confundir dimensiones
semánticas diferentes y conserva la apertura ante una pregunta mal tipada.

### Explicación y uso efectivo

Responder ya no es una lectura neutra. `genesis_education_answer_explain`
devuelve el hecho o la regla elegida, sus condiciones, el soporte vigente, la
entrada del diccionario y su contador de usos. Cuando existe una salida
resuelta, la entrada seleccionada se registra mediante el mismo mecanismo
`used_in_output` de Trinity.

Una respuesta ambigua o desconocida no atribuye uso a ninguna regla. En una
respuesta literal se registra el hecho; en una transferencia se registra la
regla más específica. El contador y la recencia forman parte del snapshot, por
lo que el sistema conserva no sólo qué sabe, sino qué conocimiento ha utilizado
realmente.

La sesión fundacional explica:

```text
hamster -> casa:  1 condición, soporte 2
halcon  -> torre: 2 condiciones, soporte 2
```

## Primera ablación semántica

Se compararon tres sistemas con los mismos seis sujetos de entrenamiento, cuatro
transferencias no vistas y dos consultas cuya respuesta correcta era abstenerse:

| Sistema | Vistos | Transferencia | Abstención | Falsos positivos | Bytes activos |
|---|---:|---:|---:|---:|---:|
| Memoria literal | 6/6 | 0/4 | 2/2 | 0 | 276 |
| Inductor simbólico | 6/6 | 4/4 | 2/2 | 0 | 476 |
| Genesis + Trinity | 6/6 | 4/4 | 2/2 | 0 | 36 424 |

El inductor simbólico no recibe reglas manuales: deriva condiciones simples y
compuestas con el mismo soporte mínimo y la misma precedencia jerárquica. La
prueba incluye una regla general y una excepción más específica. Además,
Genesis produce exactamente las mismas decisiones con semillas tensoriales `42`
y `99`.

La ablación refuta por ahora una ventaja semántica atribuible a los tensores:
Genesis supera a la memoria literal, pero no supera al inductor simbólico
equivalente y utiliza una representación operacional mucho mayor. Los bytes no
incluyen capacidad reservada sin usar; suman únicamente nodos y entradas
activas. Trinity aporta en este ensayo identidad pública, procedencia, cierre y
persistencia, no una mejora predictiva medida.

El ensayo también detectó y corrigió un fallo: los contraejemplos cubiertos por
una regla más específica anulaban la regla general. Ahora una excepción válida
subordina el caso particular sin destruir la transferencia general.

Estas cifras describen un corpus mínimo construido; no estiman todavía el
comportamiento en lenguaje natural ni a escala.

## Arquitectura funcional Butterfly

El documento de transición propone cinco responsabilidades. Su relación con el
estado actual es:

| Responsabilidad | Proyecto | Estado |
|---|---|---|
| `ffe_encoder` | Genesis G-E | Pendiente |
| Transcender | Trinity | Implementado en el núcleo y semilla ejecutiva |
| Evolver | Genesis sobre Trinity | Parcial: diccionario/LRU; faltan arquetipos y dinámicas |
| Harmonizer | Trinity + currículo Genesis | Parcial: configurable; calibración aprendida pendiente |
| Extender | Trinity/Genesis | Parcial: procedencia y backoff; texto natural pendiente |

MCP puede utilizarse posteriormente para desplegar estas responsabilidades
como servicios independientes, pero no forma parte del contrato matemático ni
es un requisito del primer experimento local.

## Hipótesis, no conclusiones

Las siguientes proposiciones del marco Butterfly deben tratarse como hipótesis
experimentales:

- los tensores son más interpretables que los embeddings;
- preservan suficiente semántica con mucha menos memoria;
- reducen el coste de cómputo total;
- permiten aprendizaje continuo sin degradación catastrófica;
- reorganizan conocimiento en vez de limitarse a memorizar;
- conservan capacidades relevantes del modelo profesor.

Las referencias a inteligencia viva, esencia cognitiva o conciencia pertenecen
por ahora al marco filosófico. Los experimentos medirán continuidad operativa,
autoorganización, memoria y adaptación, sin convertir esas métricas en una
afirmación automática de conciencia.

## Bloqueo de especificación antes del encoder

Butterfly propone tres niveles de 3, 9 y 27 unidades. Trinity V2 ofrece una
interpretación técnica especialmente natural para los 117 bits:

```text
3 + 9 + 27 = 39 celdas de dominio
cada dominio = máscara no vacía de 3 bits sobre {0,1,2}
39 × 3 = 117 bits
```

Este sería el **payload semántico canónico**, no el tamaño total de un tensor
operacional ni de un snapshot. Antes de implementar `ffe_encoder` todavía hay
que fijar:

- si son trits, dominios, tripletas o SO;
- cómo se relacionan FFE (Form–Function–Structure) y `FO/FN/ES` de Trinity;
- cómo se proyectan esas 39 máscaras sobre la SO y el tensor de Trinity;
- qué partes incluyen `DE`, `DO` y procedencia;
- cuál es la profundidad canónica de un token simple.

El orden físico seguirá el contrato de Trinity (`ES`, `FN`, `FO`). FFE describe
los papeles Form–Function–Structure, pero no introduce un segundo orden de
almacenamiento. La procedencia, `DE`, `DO`, estados y diccionario se contabilizan
separadamente.
