U: la salida abierta de Aurora

Cómo una misma estructura puede convertirse en tensor emergente o continuar como carry

En una ventana de Aurora aparecen tres tensores:

[W=(A,B,U;O_U)]

A primera vista, esta expresión puede sugerir que los tres cumplen la misma función. Sin embargo, existe una diferencia fundamental. (A) y (B) son las estructuras que entran en relación, mientras que (U) es la posición en la que esa relación intenta construir una salida.

Por eso (U) comienza completamente abierto:

[U_0=(2,2,\ldots,2)]

El valor (2) no representa aquí un dato vacío que deba rellenarse de forma arbitraria. Representa un espacio todavía no determinado: una salida posible cuya forma deberá emerger de la relación entre (A), (B) y el conocimiento recuperado del diccionario.

Esta interpretación permite comprender una propiedad decisiva de Aurora: la salida consolidada y el carry no son dos estructuras independientes. Ambos proceden de la evolución del mismo tensor abierto (U). Lo que cambia es el estado alcanzado por la ventana.

La orientación pertenece a U

Cada salida abierta se construye bajo una orientación de ventana propia:

[O_U=(o_{ES},o_{FN},o_{FO})]

Conviene distinguirla de (DO). (DO_t) es el estado del recorrido que ordena y contabiliza el intento actual; (O_U) es la orientación efectiva que queda asociada a la salida producida por ese intento:

[DO_t\longrightarrow O_U\longrightarrow U^*]

Una U sin historia comienza en la orientación origen ((0,0,0)). En una reejecución, (O_U) toma el estado (DO_t). Si (U^*) se convierte en tensor emergente o en carry, conserva esa orientación. Cuando el carry ocupa (A) en la ventana siguiente, la orientación se hereda y no se reinicia silenciosamente.

La ventana como relación productora de una salida

La operación puede expresarse de forma simplificada como:

[(A,B,U_0;K)\longrightarrow U^*]

donde:

(A) y (B) son los tensores que se están relacionando;

(U_0) es la salida inicialmente abierta;

(K) representa el conocimiento recuperado del diccionario;

(U^*) es el estado alcanzado por la salida después de operar la ventana.

El diccionario no sustituye a (U). Su función es proporcionar estructuras candidatas capaces de orientar su resolución. Estas candidatas se reejecutan dentro de la relación actual; no se aceptan únicamente porque hayan cerrado anteriormente.

Por tanto, el proceso no consiste en copiar un tensor almacenado sobre una salida vacía. Consiste en comprobar si una estructura aprendida permite cerrar la relación concreta formada por (A), (B) y (U).

Una vez alcanzado (U^*), la dimensión de coherencia de la ventana determina su destino:

[DE=\begin{cases}0 & \text{coherencia}\1 & \text{incoherencia}\2 & \text{ambigüedad}\end{cases}]

Cuando la ventana cierra: U se consolida como salida

Si la ventana alcanza:

[DE=0]

la relación ha cerrado. El vector residual es nulo y (U^*) puede consolidarse como resultado de la ventana.

Sin embargo, el tensor emergente no debería reducirse a una copia desnuda de los trits alcanzados por (U). Para que la estructura pueda verificarse, reutilizarse y extenderse posteriormente, debe conservar también la información que explica cómo fue producida.

Podemos representarlo así:

[T_E=\operatorname{encapsular}(U^*,A,B,K,O_U,DO,\Pi)]

donde:

(U^*) contiene la salida calculada;

(A) y (B) conservan las estructuras de origen;

(K) identifica el conocimiento utilizado;

(O_U) conserva la orientación con la que se construyó la salida;

(DO) conserva el recorrido y el gasto de búsqueda;

(\Pi) conserva la procedencia completa.

No significa necesariamente que todos estos elementos deban duplicarse físicamente dentro de (T_E). Pueden conservarse mediante referencias, relaciones compartidas o enlaces al grafo del diccionario. Lo importante es que la síntesis no destruya la posibilidad de reejecutar su propio origen.

El recorrido sería:

[U_0\xrightarrow[\text{con }A,B,K]{\text{operación}}U^*\xrightarrow{DE=0}T_E]

Una vez consolidado, (T_E) puede:

ascender al nivel fractal superior;

entrar en el diccionario como estructura reutilizable;

participar en nuevas ventanas;

actuar como índice de sus relaciones inferiores;

extenderse posteriormente para recuperar las estructuras que lo originaron.

De esta forma, el resultado no es solamente una respuesta. Es una nueva unidad de conocimiento operativo.

Cuando la ventana permanece abierta: U se convierte en carry

Si la ventana alcanza:

[DE=2]

la relación no es falsa, pero tampoco dispone todavía de información suficiente para producir una síntesis cerrada.

En ese caso, (U^*) no debe descartarse. La operación ya puede haber resuelto parte de sus dimensiones, seleccionado determinadas orientaciones y eliminado algunas alternativas incompatibles. Ese avance debe conservarse.

El tensor evolucionado se convierte entonces en carry:

[T_C=\operatorname{encapsular}(U^*,A,B,K,O_U,DO,\Pi)]

La diferencia entre (T_E) y (T_C) no reside necesariamente en su forma, sino en su estado lógico:

[\begin{aligned}T_E &: \text{relación cerrada convertida en unidad},\T_C &: \text{relación abierta que necesita más contexto}.\end{aligned}]

El carry pasa a ocupar la posición (A) de la ventana siguiente:

[(A_i,B_i,U_i;O_{U_i})\longrightarrow(T_{C_i}^{O_{U_i}},T_{i+1},U_0)]

Este es el movimiento horizontal de Aurora. La relación todavía no asciende como un nuevo objeto consolidado, sino que continúa avanzando por el clúster para incorporar otro tensor y volver a intentar el cierre.

El carry no es, por tanto, un resto sobrante ni una señal auxiliar. Es la salida parcialmente construida que todavía no ha conseguido convertirse en objeto.

Por qué el carry debe conservar la procedencia

Transportar únicamente los trits actuales de (U^*) podría provocar una pérdida de información.

Dos ventanas distintas podrían producir el mismo patrón ternario parcial y, sin embargo, haber llegado hasta él mediante relaciones diferentes. Si el sistema olvidara qué tensores participaron, qué conocimiento fue utilizado y qué caminos ya se probaron, no podría distinguir ambos casos ni retroceder de forma localizada.

Por ello, el carry debe conservar al menos:

el estado completo alcanzado por (U);

las relaciones que permanecen abiertas;

la procedencia de (A) y (B);

la orientación vigente;

las candidatas ya ensayadas;

el gasto o recorrido realizado;

la información necesaria para reejecutar el proceso.

Esta conservación no obliga a copiar toda la historia en cada ventana. La estructura fractal y el uso de referencias compartidas permiten que el carry mantenga su procedencia sin crecer necesariamente como una lista plana e ilimitada.

La cuestión computacional no es si debe conservarse la procedencia —es necesaria—, sino cómo representarla de la forma más compacta posible.

La incoherencia no es ambigüedad

Existe una distinción fundamental entre una relación abierta y una relación contradictoria.

Si la ventana alcanza:

[DE=1]

la salida ensayada no es compatible con la relación. Pero una contradicción producida por una candidata concreta del diccionario no debe declarar inmediatamente incoherente toda la ventana.

Primero deben probarse las alternativas admisibles siguiendo su prioridad. Si una candidata falla, Aurora retrocede hasta la bifurcación correspondiente y ensaya la siguiente. La prioridad decide qué ruta se intenta antes, pero no elimina las demás.

Solo cuando la incoherencia es concluyente —porque ninguna candidata compatible permite cerrar dentro del recorrido disponible— la ventana rechaza esa síntesis:

[(A_i,B_i,U_i)longrightarrow A_i\uparrow]

y continúa con:

[(B_i,T_{i+1},U_0)]

En este caso, (U^*) no se transforma en carry. El carry expresa una posibilidad todavía abierta; la incoherencia concluyente expresa que esa relación no debe continuar como una unidad.

Movimiento horizontal y movimiento vertical

La doble función de (U) permite describir los dos movimientos fundamentales de Aurora.

Cuando la salida permanece abierta, el proceso se desplaza horizontalmente:

[U^*\xrightarrow{DE=2}T_C\rightarrow\text{ventana siguiente}]

Cuando la salida cierra, el proceso se desplaza verticalmente:

[U^*\xrightarrow{DE=0}T_E\rightarrow\text{nivel superior}]

Así, una misma estructura puede pasar de ser espacio abierto a relación parcial y, finalmente, a objeto emergente:

[\boxed{U_0\rightarrow U^*\rightarrow\begin{cases}T_E & \text{si cierra},\T_C & \text{si permanece abierto}.\end{cases}}]

En la frontera del clúster, si ya no existe otro tensor capaz de proporcionar contexto, el carry puede ascender al ciclo superior como unidad abierta. Pero debe conservar explícitamente ese estado: ascender no lo convierte automáticamente en una síntesis coherente.

La misma operación en todas las escalas

Esta interpretación refuerza la autosimilitud de Aurora.

En un TriGate puede existir un resultado (R) abierto. En el transcender existe una salida (S) abierta. En la ventana tensorial existe un tensor (U) abierto. En todos los casos aparece la misma lógica:

[\text{posición abierta}\rightarrow\text{operación relacional}\rightarrow\text{cierre, contradicción o continuidad}]

La diferencia no reside en el operador, sino en la escala de la estructura operada.

Un trit abierto puede resolverse como resultado de un TriGate. Una tripleta abierta puede resolverse como salida de una cara. Un tensor abierto puede resolverse como salida de una ventana. Si la información todavía no basta, la estructura conserva su evolución y busca contexto adicional.

Por eso (U) no debería describirse como una capa externa de conocimiento ni como un tercer token observado. Es la reproducción, a escala tensorial, de la salida abierta que ya existe en las relaciones inferiores.

Una interpretación cercana a la satisfacción de restricciones

El mecanismo puede entenderse también como un sistema relacional de satisfacción de restricciones.

Los tensores (A) y (B) establecen una relación. El diccionario recupera estructuras aprendidas que podrían satisfacerla. Cada candidata se reejecuta dentro del contexto actual y transforma progresivamente la salida abierta (U).

Entonces pueden ocurrir tres cosas:

Existe una configuración que satisface la relación: la salida cierra y emerge.

Existen varias configuraciones posibles o falta información: la salida permanece abierta y continúa como carry.

Ninguna configuración admisible satisface la relación: la síntesis se rechaza y los tensores continúan separados.

La inteligencia global no necesita que un controlador central conozca de antemano la respuesta. Cada relación local reduce su pequeño espacio de posibilidades y propaga los cambios a las relaciones vecinas. La redundancia permite que el conjunto vaya reorientando las decisiones locales hasta encontrar una configuración compatible.

Una distinción necesaria: contenido, estado y unidad

Para evitar ambigüedades, conviene distinguir tres conceptos:

Símbolo

Función

(U^*)

Contenido alcanzado por la salida durante la operación

(T_E)

Unidad cerrada que encapsula (U^*) y su procedencia

(T_C)

Unidad abierta que encapsula (U^*), su procedencia y el estado pendiente

Esta distinción no obliga a crear tres objetos físicos completamente diferentes. (T_E) y (T_C) pueden compartir la misma representación tensorial y diferenciarse mediante su dimensión de coherencia y su posición dentro del ciclo.

Lo importante es no confundir el contenido calculado con la condición lógica que permite usarlo.

Una hipótesis que debe verificarse

La función dual de (U) es conceptualmente coherente: evita tanto forzar una conclusión prematura como descartar el trabajo realizado por una relación todavía abierta.

Sin embargo, su eficacia no puede darse por demostrada únicamente a partir de la arquitectura. La implementación deberá comprobar al menos cuatro propiedades:

que el carry conserva toda la información necesaria para cerrar posteriormente;

que su representación no crece sin límite;

que distintas secuencias de activación convergen cuando existe una solución única;

que el coste de transportar y reejecutar el carry resulta menor que reconstruir desde cero todo el espacio de posibilidades.

Estas preguntas no invalidan el mecanismo. Lo convierten en una hipótesis precisa y experimentalmente evaluable.

Conclusión

El tensor (U) es la salida abierta de la ventana de Aurora.

Comienza como un espacio completamente desconocido y evoluciona bajo las restricciones producidas por (A), (B) y el conocimiento recuperado del diccionario. Si la relación cierra, su estado se consolida como tensor emergente y puede ascender, almacenarse y extenderse. Si la relación permanece ambigua, el mismo estado continúa como carry y busca nuevo contexto. Si la relación resulta concluyentemente incoherente, esa salida se abandona sin forzar una síntesis falsa.

La idea puede resumirse así:

[\boxed{U\text{ no es a veces salida y a veces carry.}}]

[\boxed{U\text{ es siempre la salida en construcción;}\\text{el cierre la convierte en emergencia y la apertura en carry.}}]

Esta distinción convierte la ventana en algo más que un mecanismo de combinación. La convierte en un proceso capaz de decidir cuándo una relación ya puede existir como objeto y cuándo todavía debe continuar buscando su forma.
