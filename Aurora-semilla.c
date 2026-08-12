/* ============================================================================
 * AURORA ASYNC v0.1 — sustrato experimental de propagación local
 * ============================================================================
 * Los trigates NO intercambian copias: comparten celdas ternarias por índice.
 * Cuando una celda cambia, se encolan los trigates suscritos. Cada trigate
 * ejecuta su regla LOCAL (consistencia bidireccional sobre sus 4 celdas).
 * No hay coordinador global: la coordinación emerge de decisiones locales,
 * y el estado del sistema emerge como el patrón de E de sus trigates.
 *
 * SIGNIFICADO CONTEXTUAL DEL 2: el valor 2 representa lo que no puede
 * resolverse dentro de la distinción activa. Su significado completo no
 * reside en el trit aislado: emerge del cierre contextual (la cara C5 del
 * Transcender relaciona los DE de entrada, conocimiento y salida). Este
 * sustrato propaga valores y restricciones; la interpretación semántica
 * definitiva pertenece a las capas superiores.
 *
 * PROPIEDAD: la propagación es poda monótona sobre un retículo finito, de lo
 * que se deriva un punto fijo de propagación único e independiente del orden
 * de eventos (ese punto fijo puede conservar relaciones abiertas, E=2).
 * La demo [4] aporta la comprobación experimental de dicha independencia.
 * ============================================================================ */
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MAXC 128
#define MAXT 128
#define MAXSUB 32
#define QCAP 4096

typedef struct { uint8_t v; int nsub; int sub[MAXSUB]; char nombre[16]; } Celda;
typedef struct { int a, b, m, r; uint8_t E; } Trigate;

static Celda C[MAXC];  static Trigate T[MAXT];
static int nc, nt;
static int cola[QCAP], qh, qn;          /* anillo: cabeza + pendientes */
static uint8_t en_cola[MAXT];
static int contradiccion, desbordado, verboso = 1;

static uint8_t maj3(uint8_t a, uint8_t b, uint8_t m) {
    if (a == b || a == m) return a;
    if (b == m) return b;
    return 2;
}
static void reset(void) {
    nc = nt = qh = qn = contradiccion = desbordado = 0;
    memset(en_cola, 0, sizeof en_cola);
}
static int celda(const char *nom) {
    if (nc >= MAXC) { fprintf(stderr, "limite MAXC\n"); exit(1); }
    C[nc].v = 2; C[nc].nsub = 0;
    snprintf(C[nc].nombre, 16, "%s", nom);
    return nc++;
}
static void suscribir(int c, int t) {
    if (C[c].nsub >= MAXSUB) { fprintf(stderr, "limite MAXSUB en %s\n", C[c].nombre); exit(1); }
    C[c].sub[C[c].nsub++] = t;
}
static void encolar(int t) {
    if (en_cola[t]) return;
    if (qn >= QCAP) { desbordado = 1; return; }   /* nunca se pierde en silencio */
    en_cola[t] = 1; cola[(qh + qn) % QCAP] = t; qn++;
}
static int trigate(int a, int b, int m, int r) {
    if (nt >= MAXT) { fprintf(stderr, "limite MAXT\n"); exit(1); }
    T[nt] = (Trigate){a, b, m, r, 2};
    int id = nt++;
    suscribir(a, id); suscribir(b, id); suscribir(m, id); suscribir(r, id);
    return id;
}
/* escribir: el UNICO punto donde nace la coordinacion.
 * Refina una incognita hacia un valor definido (no "afirma el concepto 2").
 * Un choque definido/definido es un cruce: se marca, no se sobrescribe. */
static void escribir(int c, uint8_t val, const char *causa) {
    if (val == 2 || C[c].v == val) return;
    if (C[c].v != 2) {
        if (verboso) printf("    !! CRUCE en %s: %d vs %d (%s)\n",
                            C[c].nombre, C[c].v, val, causa);
        contradiccion = 1; return;
    }
    C[c].v = val;
    if (verboso) printf("    %s := %d   <- %s\n", C[c].nombre, val, causa);
    for (int k = 0; k < C[c].nsub; k++) encolar(C[c].sub[k]);
}
/* la instruccion local: consistencia bidireccional sobre las 4 celdas,
 * respetando alias (dos puertos sobre la misma celda = el mismo valor) */
static void ejecutar(int ti) {
    Trigate *t = &T[ti];
    uint8_t va = C[t->a].v, vb = C[t->b].v, vm = C[t->m].v, vr = C[t->r].v;
    int sA[3] = {0}, sB[3] = {0}, sM[3] = {0}, sR[3] = {0}, n = 0;
    for (uint8_t a = 0; a < 3; a++) {
        if (va != 2 && a != va) continue;
        for (uint8_t b = 0; b < 3; b++) {
            if (vb != 2 && b != vb) continue;
            if (t->b == t->a && b != a) continue;              /* alias */
            for (uint8_t m = 0; m < 3; m++) {
                if (vm != 2 && m != vm) continue;
                if (t->m == t->a && m != a) continue;
                if (t->m == t->b && m != b) continue;
                uint8_t r = maj3(a, b, m);
                if (vr != 2 && r != vr) continue;
                if (t->r == t->a && r != a) continue;
                if (t->r == t->b && r != b) continue;
                if (t->r == t->m && r != m) continue;
                sA[a]++; sB[b]++; sM[m]++; sR[r]++; n++;
            } } }
    if (n == 0) {
        t->E = 1; contradiccion = 1;
        if (verboso) printf("    !! trigate#%d: E=1 — relacion imposible\n", ti);
        return;
    }
    char causa[32]; snprintf(causa, 32, "trigate#%d", ti);
    int *sup[4] = {sA, sB, sM, sR};
    int cel[4]  = {t->a, t->b, t->m, t->r};
    for (int k = 0; k < 4; k++) {
        if (C[cel[k]].v != 2) continue;
        int unico = -1, cuenta = 0;
        for (int v = 0; v < 3; v++) if (sup[k][v]) { unico = v; cuenta++; }
        if (cuenta == 1) escribir(cel[k], (uint8_t)unico, causa);
    }
    uint8_t antes = t->E;
    t->E = (C[t->a].v != 2 && C[t->b].v != 2 && C[t->m].v != 2 && C[t->r].v != 2) ? 0 : 2;
    if (t->E == 0 && antes != 0 && verboso)
        printf("    ** trigate#%d cierra: E=0 (vector residual nulo)\n", ti);
}
/* propagar hasta el punto fijo de propagacion (puede conservar E=2 abiertos).
 * orden!=0 -> se consume la cola en orden aleatorio (independencia de orden) */
static void propagar(unsigned orden) {
    if (orden) srand(orden);
    while (qn > 0 && !contradiccion && !desbordado) {
        if (orden) { int j = rand() % qn;
                     int ia = qh % QCAP, ib = (qh + j) % QCAP;
                     int tmp = cola[ia]; cola[ia] = cola[ib]; cola[ib] = tmp; }
        int t = cola[qh % QCAP]; qh++; qn--; en_cola[t] = 0;
        ejecutar(t);
    }
    if (desbordado) fprintf(stderr, "cola desbordada: propagacion incompleta\n");
}
static void despertar_todo(void) { for (int t = 0; t < nt; t++) encolar(t); }

int main(void) {
    printf("AURORA ASYNC v0.1 — coordinacion local, emergencia de estados\n");
    printf("==============================================================\n");

    printf("\n[1] La inversa: OR con R=0 resuelve dos incognitas totales\n");
    reset();
    int A = celda("A"), B = celda("B"), M = celda("M"), R = celda("R");
    trigate(A, B, M, R);
    escribir(M, 1, "percepcion"); escribir(R, 0, "percepcion");
    propagar(0);

    printf("\n[2] Cascada entre trigates que comparten celdas\n");
    reset();
    int a = celda("a"), b = celda("b"), m1 = celda("m1"), r1 = celda("r1");
    int c2 = celda("c"),  m2 = celda("m2"), r2 = celda("r2");
    trigate(a, b, m1, r1);
    trigate(r1, c2, m2, r2);          /* r1 compartida: el puente */
    escribir(m1, 1, "percepcion");
    escribir(a, 0, "percepcion");
    escribir(b, 0, "percepcion");     /* OR(0,0)=0 -> r1=0 */
    escribir(m2, 1, "percepcion"); escribir(r2, 1, "percepcion");
    propagar(0);                      /* r1=0 y OR=1 -> c=1 hacia atras */

    printf("\n[3] Cruce detectado localmente\n");
    reset();
    int x = celda("x"), y = celda("y"), mm = celda("mm"), rr = celda("rr");
    trigate(x, y, mm, rr);
    escribir(x, 1, "percepcion"); escribir(y, 1, "percepcion");
    escribir(mm, 0, "percepcion"); escribir(rr, 0, "percepcion"); /* AND(1,1)=1 != 0 */
    propagar(0);

    printf("\n[4] Independencia del orden: 40 redes x 6 ordenes de eventos\n");
    verboso = 0;
    int confluentes = 0, total = 0;
    for (unsigned red = 1; red <= 40; red++) {
        uint8_t final[8][MAXC]; int ncc = 0, contra[8];
        for (unsigned orden = 0; orden < 6; orden++) {
            reset(); srand(red * 7919);
            for (int i = 0; i < 14; i++) { char nom[8]; snprintf(nom,8,"c%d",i); celda(nom); }
            for (int i = 0; i < 9; i++) {
                int q[4]; for (int k = 0; k < 4; k++) q[k] = rand() % nc;
                trigate(q[0], q[1], q[2], q[3]);
            }
            for (int i = 0; i < 5; i++) escribir(rand() % nc, rand() % 2, "semilla");
            despertar_todo();
            propagar(orden ? orden * 131 + red : 0);
            for (int i = 0; i < nc; i++) final[orden][i] = C[i].v;
            contra[orden] = contradiccion; ncc = nc;
        }
        int igual = 1;
        for (unsigned o = 1; o < 6; o++) {
            if (contra[o] != contra[0]) igual = 0;
            if (!contra[0] && memcmp(final[o], final[0], ncc)) igual = 0;
        }
        total++; confluentes += igual;
    }
    printf("    redes con punto fijo de propagacion identico: %d/%d\n", confluentes, total);
    printf("\n(propiedad derivada de la poda monotona sobre reticulo finito;\n"
           " esta demo aporta la comprobacion experimental de la independencia\n"
           " respecto al orden de eventos)\n");
    return 0;
}
