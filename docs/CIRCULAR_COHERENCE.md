# Coherencia circular exacta

El TriGate comprime localmente: una salida determinada no implica una
interpretación interna única. Trinity distingue ahora de forma operacional:

- contradicción global: cero configuraciones compatibles;
- cierre interpretativo: una configuración compatible;
- apertura global: más de una configuración compatible.

`aurora_circular_solve` recibe dominios de variables y restricciones
`R = Majority3(A,B,M)`. Enumera los modelos globales exactos y devuelve tanto
su número como la unión de valores soportados para cada variable.

## La conclusión global no se compone desde decisiones locales

La red no contiene primero varias decisiones locales para organizarlas después.
Todos los TriGates expresan simultáneamente restricciones parciales sobre una
misma configuración. El conjunto coherente nace de su conjunción directa:

```text
G = { S | R1(S) AND R2(S) AND ... AND Rn(S) }
```

Por tanto, la conclusión global no es una mayoría de mayorías, una votación, un
consenso ni una jerarquización de salidas locales. Una salida local puede estar
determinada y seguir siendo compatible con varias configuraciones de `G`.
También puede parecer cerrada dentro de su gate sin que la red completa posea
una interpretación única.

La propagación local tiene un papel computacional, no constitutivo: elimina
valores que carecen de soporte local sin elegir una interpretación. El solver
global se define siempre sobre las restricciones originales simultáneas. Puede
ejecutarse después de la poda por eficiencia únicamente cuando esa poda ha sido
demostrada conservadora; su verdad no procede del orden ni de las decisiones de
esa fase.

## Casos de referencia

### Cierre operativo, apertura interpretativa

```text
Majority3(1, 1, M) = 1, M in {0,1,2}
```

La salida está fijada, pero existen tres modelos. El estado global es abierto y
el dominio soportado de `M` continúa siendo `{0,1,2}`.

### Determinación circular global

```text
r1 = Majority3(0,1,r3)
r2 = Majority3(0,2,r1)
r3 = Majority3(1,2,r2)
```

Cada pareja fija contiene valores distintos y necesita su modo. Al exigir las
tres ecuaciones simultáneamente sólo sobrevive:

```text
(r1,r2,r3) = (2,2,2)
```

La red posee exactamente un modelo y alcanza cierre interpretativo.

### Contradicción

```text
Majority3(1,1,0) = 0
```

No existe modelo compatible.

## Límite de la afirmación

La circularidad no recupera automáticamente toda información comprimida. Sólo
revela un valor cuando la red contiene restricciones independientes suficientes
para eliminar las alternativas. Si varias configuraciones sobreviven, la
apertura es real; si ninguna sobrevive, existe contradicción.

El solver exacto está limitado a 12 variables y 32 TriGates. Es un oráculo de
referencia para verificar redes pequeñas, no todavía el algoritmo escalable de
propagación. Su función inmediata es comparar futuros razonadores iterativos
contra el conjunto global verdadero y detectar puntos fijos engañosos.

## Evaluación frente a propagación local

Se comparó `aurora_network_propagate` con el conjunto global exacto.

En los 250 047 anillos regulares formados por seis entradas singleton y los
siete dominios posibles para cada uno de los tres resultados, ambos métodos
coinciden por completo:

| Estado | Redes |
|---|---:|
| Contradicción | 196 279 |
| Interpretación única | 52 696 |
| Apertura | 1 072 |

No hubo valores adicionales que podar. Para esta familia, las proyecciones
obtenidas localmente coinciden con las proyecciones del conjunto global. Esto es
una equivalencia de resultados, no una construcción de lo global desde lo
local.

Una muestra determinista (`seed=42`) de 10 000 redes más diversas, con seis
variables, dominios arbitrarios y cuatro TriGates, mostró la frontera:

| Medida global adicional | Casos |
|---|---:|
| Contradicciones no detectadas localmente | 77 |
| Interpretaciones únicas dejadas abiertas | 114 |
| Valores adicionales eliminados | 2 515 |
| Cierres locales sin unicidad global | 54 |
| Valores válidos eliminados por la poda local | 0 |

La conclusión es que la propagación local es segura en la muestra, pero no
completa en topologías generales. La coherencia global no es una mejora
organizativa de esa propagación: es el objeto simultáneo contra el que se mide.
En los anillos regulares ambas proyecciones coinciden; en redes menos uniformes,
el conjunto global revela aperturas, contradicciones y cierres que ninguna
colección de estados locales permite identificar por separado.
