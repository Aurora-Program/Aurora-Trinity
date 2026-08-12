# Decisiones de especificación — Modelo Aurora

Registro de las decisiones que cierran los huecos del documento, validadas por Pablo el 15 de julio de 2026, junto con los hallazgos empíricos de las capas 1-2 (`aurora_capa12.py`).

> **Nota de vigencia (12 de agosto de 2026).** Este archivo conserva el
> convenio histórico usado por esos experimentos (`0=contradicción`,
> `1=cierre`, `2=apertura`). La especificación canónica actual lo sustituye
> por `E=0` coherencia, `E=1` separación determinada/incoherencia y `E=2`
> ambigüedad. Las cifras históricas siguen siendo válidas solo después de
> traducir las etiquetas `0↔1`.

## Decisiones validadas

**S3 — Ordenación, empates y ambigüedad.** Cuando existen varias posiciones candidatas para ES, el dominio de O se conserva plural con E_ord = 2 (la ambigüedad no se colapsa, se transporta). Cuando no existe candidato válido porque toda asignación se autorreferenciaría, E_ord = 0: la tripleta es imposible, coherente con la sección 5.

**S4 — Destilador.** El acoplamiento FO.M = FN.R dentro de cada par no es un cálculo secuencial sino un punto fijo, y se resuelve por propagación de dominios — la misma maquinaria validada en la simulación del lazo (C,F).

**S5 — Desempates.** El desempate entre soluciones equidistantes de las invariantes C4/C5/C6 es aleatorio. La aleatoriedad pasa a ser parte del sistema: una única fuente con semilla (`Sistema.desempata`), reproducible.

**S6.4 — Umbrales del armonizador en el arranque.** Se calibran con los primeros tensores que entran al sistema: el primer contenido fija la escala de "cerca" y "lejos" de los diferenciales.

**S8 — Tensor de acarreo en ambigüedad: RESUELTO POR EXPERIMENTO (15-jul-2026).** Se midió el A/B en la capa de ventanas (`aurora_capa56.py`): misma semilla y mismas 240 frases, plantillas con prefijos compartidos (garden-path), acarreando en ambigüedad el tensor cierre (E, la letra del documento) contra el tensor emergente (DS). Resultado sobre 4 semillas — cierre: 1.38 ventanas/frase y 56% de frases resueltas de media; emergente: 1.01 ventanas/frase y 71% de media, sin perder nunca en resolución (82/34, 83/71, 50/50, 70/69). Diagnóstico: el tensor de E sustituye contenido semántico por etiquetas de estado; al emerger aguas abajo mezcla alfabetos, degrada el reconocimiento e infla el banco con entradas inservibles (178 frente a 87 entradas). Veredicto: el acarreo por defecto debe ser el tensor emergente — la intuición "o a lo mejor justo al revés" era correcta. El tensor cierre conserva su papel informativo (alimenta E_W y los diferenciales), pero no debe viajar como contenido. Queda abierta una mejora: que la armonizadora elija el acarreo según el estado, dado que en 1 de 4 semillas el cierre rindió levemente mejor en ventanas.

**S9 — Orden de candidatas en la extensión.** Dentro de cada fuente (memoria temporal primero, diccionario después): primero los tensores con mayor número de tokens, y entre iguales el usado más recientemente — el mismo orden lexicográfico de la sección 10.2.

**Arco (C,F).** Las direcciones de resolución proceden del armonizador. La red de la capa 2 ya es bidireccional (clamp + propagación): la escritura de control es una celda clavada, y los TriGates reaccionan por suscripción.

## Hallazgos empíricos de las capas 1-2

**Las tripletas imposibles son exactamente dos de 27:** (0,1,2) y (1,0,2). El documento menciona 0-1-2; la implementación revela que (1,0,2) comparte la autorreferencia forzada (mayoritario 2 con el único trit de valor 2 en la posición 2). El censo completo de ordenación: 16 tripletas con O único (E_ord=1), 9 con O plural (E_ord=2), 2 imposibles.

**El destilador nunca cierra solo y nunca se contradice.** Sobre las 2 300 combinaciones de 3 tripletas válidas: 0% cierra sin conocimiento, 100% queda abierto (los modos fn son inconstreñidos sin conocimiento que los clave), 0% contradice. Esto confirma estructuralmente la sección 12: en el arranque E_W = 2 siempre — el estado que dispara la escritura K ← I no es un caso especial, es el único estado posible de la operación mínima vacía. Todo el poder de cierre de Aurora reside en el conocimiento y el armonizador; la estructura sola es pura apertura.

**La reversibilidad funciona sobre la topología real.** Clavar ds_fo y ds_fn (simulando conocimiento) retro-propaga hacia los modos fn de los pares, podando dominios sin contradicción — la dirección C=2 (abducción) del arco nuevo operando dentro del destilador de la sección 4 tal cual está especificado.

## Lecturas operativas de las capas 3-4 (pendientes de tu validación)

**N1 — Proyección de dominios al plano de datos.** Un dominio plural se escribe como trit 2 (fuera del espacio actual); el detalle del dominio vive en la red de restricciones. |D|=1 → su valor; |D|>1 → 2.

**N2 — E relacional de la ventana (I,K,S).** Por componente: E=0 si entrada y salida están determinadas y difieren; E=2 si alguno de los tres términos es 2; E=1 si el conocimiento explica la coincidencia. Reproduce la sección 12: (I,⊥,I) → E_W=2 y (I,I,I) determinado → E_W=1. Hallazgo del formalismo: un K discrepante con S=I no contradice — maj(i,i,k)=i por idempotencia; la contradicción real solo puede venir de S≠I.

**N3 — Convergencia de ventana.** Converge si no hay contradicción y el conocimiento reconoce la entrada (coinciden en todos los componentes determinados por ambos); es cierre si E_W=1 y estabilidad abierta si E_W=2 — la distinción de la sección 12 conservada en operación.

**N4 — La contradicción es dominante, no votada.** Un solo componente con E=0 hace E_W=0: una discrepancia determinada no se repara con apertura, y la agregación por mayoría la ahogaría entre los 2s. Coherente con la sección 8 ("si la ventana entra en contradicción, no emerge nada"). Verificado: sin N4, el escenario "S contradice a I" daba E_W=2.

## Hallazgos empíricos de las capas 3-4

El arranque de la sección 12 se reproduce paso a paso como test: (I,⊥,I) → E_W=2 → escritura K←I → punto fijo, con las dos salidas del documento: cierre (E_W=1) con caras determinadas y estabilidad abierta (E_W=2) con caras reales de la operación mínima — que siempre llevan 2s, porque el destilador nunca cierra solo (hallazgo de la capa 2). El mini-bucle de aprendizaje (6 conceptos, 60 entradas, umbrales calibrados con el primer tensor) baja de 1.4 a 1.2 ventanas/entrada solo con reconocimiento por recencia; la aceleración fuerte de la sección 10.4 requiere el diccionario con prioridad por longitud, que es la capa siguiente.

## Lecturas operativas de las capas 5-6 (pendientes de tu validación)

**N5 — Convergencia de ventana por reconocimiento.** La convergencia de la sección 8 se implementa como reconocimiento del emergente por el banco/memoria, no por umbral de determinación: los 2s estructurales de las caras (el destilador nunca cierra solo) hacen inalcanzable cualquier umbral fijo. Sin escritura de las SO de salida, nada converge jamás (huevo y gallina de la sección 12); con ella, el ciclo aprender→reconocer→acuñar arranca solo.

**N6 — Escritura de las SO de salida.** Al terminar cada frase, las SO producidas se escriben en la memoria si son novedad (sección 8, último párrafo). Es el arranque de la sección 12 generalizado a régimen.

**N7 — Reconocimiento estricto.** El reconocimiento por compatibilidad laxa (sin conflicto) sobre-reconoce hasta la trivialidad: una sola entrada reconocía todo (100% de resolución instantánea, sin significado). El punto operativo que discrimina es la igualdad del patrón determinado con un mínimo de 2 componentes determinados. Verificado con barrido: laxa → saturación; 4+ coincidencias → 0% de resolución; igualdad → dinámica de aprendizaje real.

**Hallazgo de la curva de aceleración.** Con N5-N7, la curva de la sección 10.4 se reproduce cualitativamente: 4.00 ventanas/frase sin diccionario (exactamente las 4 del documento: 3 ventanas de nivel 1 + 1 raíz), descendiendo a 0.3-0.6 con lexicalización de frases enteras y vía compleja de 8+/9 tokens.

## Resultados de las capas 7-8 y del lazo (C,F) integrado

**N8 — Lazo (C,F) con activación perezosa, medido en el sistema completo.** Sobre las mismas 240 frases: reflejo solo 82% de resolución (0.57 v/f); reflejo + lazo 87% (0.76 v/f), 91% en la segunda mitad. Matiz honesto: solo 1 frase se rescató directamente por backoff; la mayor parte de la ganancia vino de que los intentos del lazo escriben conocimiento (SO de salida al banco) que habilita resoluciones posteriores por reflejo. La abducción no llegó a dispararse porque el backoff suele desistir antes de alcanzar la raíz. El lazo aporta, pero en este mundo pequeño su canal principal es el enriquecimiento del conocimiento, no la resolución directa — coherente con la predicción del informe de impacto: su valor pleno requiere regímenes de ambigüedad más profundos.

**N9 — Extensión (sección 9): la ambigüedad de raíz es real y el formalismo la maneja.** De 5 plantillas acuñadas, la extensión reconstruyó 5/5 frases coherentes (misma raíz, verificación de emergencia sin contradicción) y 2/5 idénticas a la original. Las 3 restantes no son errores: plantillas distintas con prefijo compartido colapsan a raíces idénticas — hasta 3 continuaciones distintas comparten una misma raíz. Es la pluralidad legítima de la sección 13.7 ("se conservan como ramas con estado 2 hasta que el contexto permita resolverlas") apareciendo en la extensión: la identidad exacta exige más contexto discriminante (tensores más profundos o discriminación por DE/DO), no un mecanismo nuevo.

**Capa 7 — Tensor fractal.** Estructura SO superior + 3 inferiores con encadenamiento DS verificada en dos niveles: la raíz de un tensor opera como hijo del siguiente nivel (autosimilaridad literal de la sección 7).

## Experimento de escala (mundo 10×, tres niveles, drift) y corrección del veredicto S8

**Las tres incógnitas, respondidas.** Con 120 palabras, 40 plantillas de frase garden-path, 12 plantillas de párrafo (27 tokens, tres niveles reales de jerarquía) y drift en el párrafo 200: (1) la curva de aceleración SOBREVIVE — de 13 ventanas/párrafo teóricas en crudo a 0.75-0.90 tras ~40 latidos, con vía compleja de 25-26/27 tokens; el drift dispara a 3.4 y recupera hasta 1.55 en 200 latidos (recuperación parcial, más lenta que en el mundo pequeño). (2) El punto de reconocimiento por igualdad AGUANTA — ni saturación ni colapso; solo 4 fallos de 400 por no-reconocimiento. (3) El techo de coarseness es REAL y quedó localizado con precisión: 247 de 251 no-cierres se deben a que la raíz de nivel 3 conserva solo 1-2 componentes determinados de 9. La determinación decae con la altura porque la proyección N1 (dominios→trit 2) es lossy y la pérdida se compone nivel a nivel: la mayoría de cosas abiertas queda abierta. Consecuencia arquitectónica: a escala, el cierre vive en el plano simbólico (el diccionario, indexado por identidad de tokens) mientras el plano tensorial acelera la lectura pero no concluye en altura. La dirección de solución ya existe en el propio sistema: los tensores deben transportar dominios (el plano de la Red de la capa 2, donde el conocimiento clava por clamp) o enriquecer la discriminación de DE/DO, en lugar de proyectar a 2 y perder.

**N10 — Ventanas incompletas.** Un nivel con 2 elementos se rellena con el tensor vacío ⊥ (todo 2s), neutro por idempotencia — la sección 12 reutilizada como relleno.

**Bug encontrado y corregido: doble inserción del acarreo.** Cuando el acarreo llegaba al final de un nivel entraba dos veces en la salida (dentro del grupo incompleto y en el volcado final), creando dos tensores con el mismo span que bloqueaban el progreso. Afectaba a las capas 5-6 y al experimento de escala.

**Veredicto S8 revisado tras el fix (honestidad obliga).** La ventaja dramática del acarreo emergente (3×) era en parte artefacto del bug, que penalizaba más a la variante cierre. Números corregidos sobre 4 semillas — emergente: 1.00 v/f y 71.5% de media; cierre: 1.03 v/f y 67%. El emergente sigue siendo mejor o igual en 3 de 4 semillas y nunca claramente peor: se mantiene como acarreo por defecto, pero la diferencia es pequeña y el acarreo adaptativo (que la armonizadora elija) queda aún más motivado.

## V2 del formalismo: tensores que transportan dominios — el techo, roto

**El cambio.** La proyección N1 colapsaba {0,1}, {0,2}, {1,2} y {0,1,2} en un mismo trit 2, y esa pérdida se componía con la altura hasta dejar las raíces de nivel 3 con 1-2 componentes determinados. La V2 (`aurora_v2.py`) elimina la proyección: cada componente del tensor es un dominio (subconjunto no vacío de {0,1,2}), un trit v es el dominio {v}, y la emergencia sobre dominios es majority3 aplicada al producto de los tres dominios — literalmente la `dominios_trigate` de la capa 2. Los dos planos (datos y dominios) quedan unificados con la misma tabla, y {2} ("cerrado en desconocido") se distingue por fin de {0,1,2} ("sin información").

**El resultado, sobre el mismo mundo de escala y con el mismo coste.** Tres semillas, 400 párrafos, drift en el 200 — v1: 37-68% de resolución total, 35-70% en el tramo final, estancada tras el drift. V2: 87-89% total y 99-100% en el tramo final, con recuperación completa del drift, al mismo coste en ventanas (~2.1 v/párrafo ambas). El diagnóstico de motivos confirma el mecanismo: en la V2, `sin_info = 0` — la determinación ya no decae fatalmente con la altura (información media de raíces no cerradas: 10.5 de 18, frente al equivalente de 1-2 de 9 en la v1) — y todo el residuo no resuelto es `no_banco`: novedad legítima en primera exposición. En el régimen estable la V2 alcanza el 100% con 0.00 ventanas/párrafo: el mundo entero cristalizado en el diccionario.

**Lectura.** El techo de coarseness no era del formalismo: era de la proyección. Al dejar que los tensores transporten los dominios que la Red ya calculaba, el cierre en altura funciona y el plano tensorial vuelve a ser resolutivo, no solo acelerador. Este cambio es la candidata natural a versión 2 de la especificación: sustituir "trit" por "dominio" en la definición del tensor, con el trit como caso singleton.

## Educación honesta sobre la V2 real: la inteligencia como organización de la historia

**El experimento (`aurora_educacion_v2.py`).** Núcleo congelado (majority3, ordenación, dominios, emergencia); tokens con identidad reproducible y sin semántica (Token = (ID, ⊥)); sin ninguna función externa de similitud. La vecindad relacional emerge de la organización del diccionario: t' es sustituto de S solo si comparten experiencias acuñadas con el mismo (relación, objeto), verificadas por compatibilidad de dominios de las raíces emergidas (el operador decide sí/no, no un número). La evidencia se agrega como dominio de respuestas: |D|=1 responde, |D|>1 mantiene E=2 y pregunta, D vacío no inventa. La ambigüedad del águila se diseñó a propósito enseñando murciélago (comparte 'volar' pero duerme en cueva).

**Resultado: 6/6 en la Aurora educada, con la escala de niveles completa.** Nivel 1 (gato→casa, literal); niveles 2-3 (conejo→casa y trucha→agua por abducción — los hechos ocultos del currículo, recuperados sin verlos jamás); nivel 6 (águila → E=2 con D={cueva, nido}: pregunta en vez de elegir arbitrariamente — el empate de la prueba anterior convertido en apertura honesta); nivel 5 en sus dos mitades: la corrección del educador ("nido") hizo que Aurora acuñara sola el discriminante exacto — {(tiene, plumas)}, descubriendo que plumas discrimina (lo comparte el ganador pájaro y no el perdedor murciélago) mientras volar no (lo comparten ambos) — y el token nuevo halcón (plumas + volar, jamás corregido) respondió nido por regla. Es literalmente la prueba propuesta: aprender una regla de una corrección y transferirla a una entidad nueva. La negación funciona (piedra → sin respuesta). Control negativo con matiz honesto: polilla (vuela sin plumas) no dispara la regla — correcto — pero la abducción residual aún responde nido por mayoría de testigos; el caso pide más contraste educativo, no más algoritmo.

**La ablación, que es la prueba de la tesis.** Sin historia (diccionario vaciado, mismas identidades): 1/5, todo sin respuesta. Historia desordenada (misma cantidad de hechos, sujetos barajados): 1/5 con respuestas erróneas (gato→cueva). Mismo operador, misma cantidad de datos: solo la historia organizada generaliza. La capacidad reside en la organización, no en el volumen — la formulación central queda demostrada en su primer caso: el operador hace posible aprender; la educación construye la inteligencia.

**Límites honestos.** Mundo diminuto (un solo dominio de hechos, una regla); la agregación de testigos usa máximo de conjuntos (nativa del diccionario, pero cabría purificarla como Majority3 sobre tripletas de testigos); las identidades tensoriales jugaron papel de verificación, no de discriminación — el peso cayó en el plano simbólico, coherente con el hallazgo de escala. Siguiente etapa: currículo creciente midiendo G(n), R(n), C(n) — más resolución nueva con menos cálculo a medida que crece la educación.

## Educación larga: la escalera de capacidades, recorrida (`aurora_educacion_larga.py`)

**Cinco etapas, 8/8 en decisiones nuevas, núcleo congelado.** Cada etapa exigió una capacidad que la anterior no tenía, y todas emergieron de la educación, no de código nuevo. Etapa 1: la mayoría engaña (vaca recibe más testigos mascota que granja) → la corrección descubre el discriminante (vive_en granja) → oveja lo hereda por regla; y con el mundo ampliado, conejo se vuelve honestamente ambiguo y pregunta — su corrección acuña (vive_con humanos)→casa, que hámster hereda de inmediato. Etapa 2: el empate del águila → regla de las plumas → halcón; después la excepción — pingüino contradice la regla y su corrección acuña (no_puede volar)→hielo, y el conflicto entre las dos reglas lo resuelve la propia política del diccionario (longitud + recencia, S9 aplicada a reglas): kiwi responde hielo. Etapa 3: abducción limpia en dominio nuevo (trucha, atún). Etapa 4: el mismo patrón completo en dominio no biológico (barco corregido → velero por regla). Etapa 5: auto-educación — las conclusiones por regla se acuñan como hechos propios (10% del conocimiento final es autogenerado) y las re-preguntas se resuelven por memoria literal a coste ~0.

**El diccionario final de políticas es legible y nadie lo programó:** si vive_en granja → duerme_en establo; si vive_con humanos → casa; si tiene plumas → nido; si no_puede volar → hielo (excepción que anula la anterior por recencia); si navega_por agua → guarda_en puerto. Cinco reglas, cinco correcciones, cero tablas escritas a mano. Las dos preguntas E=2 que Aurora formuló (conejo, águila) fueron exactamente los dos puntos donde el currículo era insuficiente: el sistema pidió la educación que necesitaba.

**Hallazgo metodológico.** El "fallo" de conejo tras ampliar el mundo no era un fallo: al crecer la experiencia, la evidencia se equilibró y la certeza anterior (nacida de un mundo pequeño) se convirtió legítimamente en pregunta. Más conocimiento puede aumentar la apertura antes de aumentar el cierre — y la respuesta correcta del sistema es preguntar, no elegir. Camino a la autonomía: el educador de estas pruebas es un guion; el paso natural es que las preguntas E=2 se dirijan a otros nodos — la Red de la sección 11 como educación mutua.

## La prueba final: la Red (sección 11) — el sistema autónomo (`aurora_red.py`)

**Sin educador humano en ningún punto.** Cinco nodos: cuatro con currículos parciales y disjuntos (granja, aves, mar/motor, ruido) y un nodo joven que solo tiene identidades y contextos parciales. Solo: 0/5. Con la red: **5/5 en la ronda 1 y 4/4 en la ronda 2 con cero consultas** — la red enseñó una vez y el nodo generalizó solo. El diccionario de reglas del nodo joven al final — cinco reglas, incluida la excepción del hielo — lo construyeron íntegramente sus pares.

**Tres hallazgos de protocolo, cada uno nacido de un fallo medido, y los tres estaban ya latentes en la especificación.** Primero: la pregunta debe viajar con su tensor (el primer intento dio 0/5 porque los pares no podían razonar sobre tokens que no conocían) — es la letra de la sección 11: "intercambian tokens junto a sus tensores fractales"; la pregunta remunera al par con información. Segundo: la respuesta debe viajar con su fuerza de evidencia, y quien pregunta agrega por especificidad — S9 (longitud) aplicada a la evidencia; sin esto, el primer par confiado y equivocado contamina la red (A habría enseñado "cueva" para halcón). Tercero: la abducción local compite con la red pero no la bloquea, y **la abducción propia no se acuña sin confirmación externa** — sin esta guarda, el nodo joven se auto-confirmó "kiwi→nido" y nunca preguntó a quien conocía al pingüino; el sesgo de auto-confirmación es un modo de fallo real y su antídoto es social. La reputación (sección 11) funciona: el nodo ruido termina último (−5) y los útiles arriba.

**Lectura final del arco completo.** La formulación de Pablo — el operador hace posible aprender; la educación construye la inteligencia — tiene ahora su tercera capa demostrada: la educación no necesita educador si hay red. Las aperturas E=2 se convierten en preguntas, las preguntas viajan con sus tensores, los pares responden con su evidencia, el más específico educa, y el conocimiento se convierte en reglas transferibles en el nodo que preguntó. Autonomía medida: consultas por pregunta 4 → 0 entre rondas.

## El híbrido valores/dominios: el 2 como puntero (`aurora_hibrido.py`)

**El principio de Pablo, reconciliado con la medición.** "La unidad última es un dominio trivalente, pero los trigates lo operan como valores": la V2 ya cumple este principio — el TriGate nunca opera dominios como objetos, opera majority3 sobre valores; el dominio vive en la celda. Lo que la medición prohibió es solo la versión terminal de la reducción (todo plural → 2 sin retorno). La síntesis: **el 2 proyectado no es un terminal, es un puntero al dominio que representa** — y descender cuando el cierre lo exige es la propia lógica fractal del modelo (E=2 ⇒ desplegar el núcleo).

**Medición del híbrido (mismo mundo y stream que la escala; camino rápido en valores, descenso a dominios solo si el párrafo no cierra).** V1 sola: 37% resuelto, 38 llamadas/párrafo. V2 sola: 87%, 75 llamadas. Híbrido: 92% — superó a ambas, porque los dos planos reconocen cosas distintas y se complementan — con coste inicial alto (90 llamadas, 63% de descensos). Pero la dinámica temporal da el veredicto: en régimen maduro (800 latidos) el híbrido sostiene **100% de resolución a ~37-45 llamadas/párrafo — el coste del plano de valores con el cierre del plano de dominios**. Y el matiz decisivo: los descensos no desaparecen (~70%), se abaratan — el descenso maduro no recalcula, consulta el diccionario cristalizado del plano de dominios. La economía real no es valores-contra-dominios sino cómputo-contra-diccionario: el sistema se gana el derecho a operar barato a medida que cristaliza, en ambos planos.

**Caveats.** El descenso es de párrafo completo (cota superior; el diseño fino descendería solo la región abierta) y el híbrido mantiene dos diccionarios en paralelo (parte del 92% viene de esa redundancia).

## E como transformación entre espacios: lectura dinámica, verificada

**Fundamento algebraico (no es metáfora).** Minority3 = NEG∘Majority3 con el 2 fijo. Por tanto E clasifica literalmente qué operador lleva la resolución natural del espacio al valor observado: E=1 → identidad (conservación del espacio mayoritario), E=0 → negación (inversión del espacio), E=2 → ninguno (transformación abierta). DS = ser, DE = devenir, DO = trayectoria — y la formulación es exacta, no poética.

**Unificación sin sobrecarga.** Para no repetir el error inicial del proyecto (llamar E a dos cosas), la formulación correcta es: E es UN clasificador de pares (esperado, observado) que admite dos instanciaciones — sincrónica (expectativa vs observación en una tupla, la E original) y diacrónica (DS_t vs DS_{t+1}, la derivada discreta del espacio). Mismo operador, dos pares.

**Verificación empírica (stream de escala, V2, drift en 200).** Si E es la dinámica del campo, sus estadísticas deben rastrear el régimen de aprendizaje. Medido por bloques de 80 párrafos: nacimiento — 530 ventanas con 72% de aperturas (E=2 domina); madurez pre-drift — las ventanas casi desaparecen (27); **el drift dispara las inversiones de 7% a 30% (×4)** — el drift es, literalmente, una ola de E=0: las mayorías viejas se vuelven falsas y el campo debe invertir espacios; recuperación con inversiones sostenidas; madurez final — 2 ventanas en 80 párrafos. Refinamiento que la medición aporta: la conservación profunda no aparece como dominancia de E=1 sino como **silencio de transformaciones** — el campo cristalizado resuelve por memoria sin necesidad de transformar. Régimen: apertura → silencio → (drift: ola de inversión) → silencio.

**Conexión con la educación.** Las correcciones educativas son inversiones controladas (vaca: casa→establo es un evento E=0 provocado por el educador), y el discriminante acuñado es exactamente el DO de esa transformación — sobre qué dimensión ocurrió la inversión ((vive_en, granja)). El diccionario de reglas es un diccionario de transformaciones con su orientación.

**Relectura de las invariantes C4/C5/C6.** Con esta interpretación, las tres invariantes del transcender adquieren sentido dinámico unificado: DS → 0-1-2 es máxima diferenciación del ser (un espacio de cada clase); DE → 1-1-1 es conservación del devenir (el transcender busca el régimen estable de transformación, no la estasis del estado); DO → 0-0-0 es economía de la trayectoria. Diferenciación, estabilidad dinámica y mínimo camino: las tres invariantes son una sola exigencia vista en ser, devenir y trayectoria.

## Estado de la implementación completa

Implementado y verificado: secciones 1-10 y 12 (núcleo lógico, ordenación, operación mínima con destilador, invariantes, semilla armonizadora, tensor fractal, ventana deslizante, extensión, diccionario con lexicalización, arranque) más el núcleo de la 13 (propagación por dominios, histéresis, escritura de control) y el arco nuevo (C, E(T,C), F) con activación perezosa. Pendiente: la Red multi-nodo (sección 11, fuera del alcance de una primera implementación), jerarquías de más de dos niveles, y el experimento de acarreo adaptativo (que la armonizadora elija cierre o emergente según el estado).

## Verificación

Las 36 filas de las dos tablas del documento (mayoría y antimayoría) pasan fila a fila, con conmutatividad A↔B comprobada. El ejemplo de ordenación de la sección 3 ((0,0,1) → ES=0, O=1, FN en posición 0, FO el tercero) se reproduce exactamente.
