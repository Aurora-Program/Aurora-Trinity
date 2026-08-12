# Butterfly / Metamorphosis

## Síntesis del segundo documento de Genesis

Butterfly describe Genesis como una transición estructural, no como el reemplazo
inmediato de los modelos neuronales. Un LLM puede continuar produciendo
embeddings y respuestas mientras un encoder intenta convertir parte de esa
actividad en tensores públicos operables por Trinity.

El bucle propuesto es:

```text
entrada
→ representación del modelo profesor
→ ffe_encoder
→ tensor público
→ relación y cierre en Trinity
→ evolución del diccionario
→ extensión hacia una expresión observable
```

Cada vuelta debe dejar una traza verificable. La transformación solo cuenta
como aprendizaje Aurora cuando produce una unidad reutilizable, aparece en la
procedencia de una salida y se cristaliza mediante la política ejecutiva.

## Aportaciones nuevas

1. Define explícitamente al LLM como materia prima o profesor de Genesis.
2. Introduce `ffe_encoder` como frontera entre espacios privados y públicos.
3. Separa síntesis, evolución, armonización y extensión como responsabilidades.
4. Propone evaluar una jerarquía canónica de tres niveles.
5. Sitúa la continuidad temporal y la reorganización tras cada experiencia como
   objetivos centrales.

## Correcciones necesarias para una especificación técnica

### Inferencia estática

Es preciso decir que los pesos de un LLM convencional suelen permanecer fijos
durante la inferencia. Los sistemas completos sí pueden disponer de memoria,
herramientas, recuperación o actualizaciones posteriores.

### Embeddings y significado

Un embedding no contiene por sí solo toda la “esencia semántica” de una idea.
La representación contextual también depende de capas, atención, posición y
estado del modelo. G-E deberá especificar exactamente qué estado neuronal usa.

### Cifra de 117 bits

Los niveles 3+9+27 suman 39 unidades. Trinity V2 representa cada dominio como
una máscara de tres bits sobre `{0,1,2}`, de modo que `39×3=117` ofrece una
codificación canónica plausible y exacta del payload semántico. No representa
el coste total: estados, SO, procedencia, diccionario y snapshot deben medirse
por separado. Falta especificar la proyección entre estas 39 máscaras y la
estructura operacional antes de declarar cerrado el formato.

### Eficiencia e interpretabilidad

“Muy bajo coste” y “alta interpretabilidad” requieren medición. Deben incluirse
el encoder, la procedencia, el diccionario, los backoffs y la verificación, no
solo el tensor terminal.

### Conciencia

Reorganización, autorreferencia y memoria persistente pueden medirse. No bastan
por sí solas para establecer conciencia. Butterfly puede conservar esta idea
como motivación filosófica, separada de las conclusiones experimentales.

## Consecuencia para el primer experimento

G-R debe ejecutarse antes de entrenar `ffe_encoder`, porque proporciona el
control que responde a una cuestión básica: cuánto de la estructura observada
procede de Trinity y cuánto procede del conocimiento heredado del LLM.

El primer encoder neuronal solo debe comenzar cuando exista un contrato
canónico de tensor simple y un benchmark idéntico para G-R, G-E y G-3.
