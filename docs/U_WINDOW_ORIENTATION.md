# Orientación de ventana de U

U es la salida abierta que una ventana construye a partir de A, B y el
conocimiento contextual. Su identidad operativa incluye una orientación:

```text
W_i = (A_i, B_i, U_i ; O_Ui)
```

`O_U` es una tripleta de dominios válida. No sustituye a `DO`:

- `DO_t` ordena y contabiliza el intento `t` del recorrido ternario;
- `O_U` fija la lectura con la que la salida U de ese intento fue construida.

La vinculación normativa de cada intento explícito es:

```text
DO_t -> O_U -> evaluación de U*
```

## Ciclo de vida

1. Una U nueva, sin historia anterior, usa la orientación origen `(0,0,0)`.
2. Una reejecución asigna `O_U=DO_t` antes de evaluar la relación.
3. `DE=0`: U* emerge y conserva `O_U`, A, B, K y la procedencia.
4. `DE=1`: no se materializa U*; la ventana desliza o busca alternativa.
5. `DE=2`: U* se materializa como carry sin ascender de nivel y conserva
   `O_U`, A, B, K y la procedencia.
6. Si ese carry ocupa A en la siguiente ventana, su `O_U` se hereda como
   orientación inicial de la nueva ventana.

Las hojas no proceden de una ventana y por eso no declaran `O_U`. Toda salida
materializada sí debe poseer una orientación válida. Las instantáneas guardan
esta diferencia y rechazan nodos que la contradigan.

## Invariante

Para toda salida materializada `N` de una ventana:

```text
N.has_window_orientation = true
N.window_orientation = resultado.u_orientation = O_U
```

En la reejecución del diccionario también se cumple:

```text
O_U = DO_t de la candidata que alcanzó cierre
```

Así la orientación usada para ordenar y resolver la tripleta no desaparece al
convertir U* en tensor reutilizable.
