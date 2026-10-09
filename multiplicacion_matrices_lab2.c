#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VAL 100

static double diferencia_segundos(struct timespec inicio, struct timespec fin) {
    return (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return 1;
    }

    int n = atoi(argv[1]);
    if (n <= 0) {
        return 1;
    }

    srand((unsigned int) time(NULL));

    /*Reserva dinamica de memoria*/
    /* BT guarda la matriz B TRASPUESTA: la fila j de BT es la columna j de B,
       es decir BT[j * n + k] = B[k][j] */
    int *A  = (int *) malloc((size_t) n * (size_t) n * sizeof(int));
    int *BT = (int *) malloc((size_t) n * (size_t) n * sizeof(int));
    int *C  = (int *) malloc((size_t) n * (size_t) n * sizeof(int));

    if (A == NULL || BT == NULL || C == NULL) {
        free(A); free(BT); free(C);
        return 1;
    }

    /* Llenado con enteros positivos aleatorios en [1, MAX_VAL].
       B se genera directamente traspuesta (lo que es fila pasa a ser columna) */
    for (int i = 0; i < n * n; i++) {
        A[i]  = 1 + rand() % MAX_VAL;
        BT[i] = 1 + rand() % MAX_VAL;
        C[i]  = 0;
    }

    /*KERNEL: unica seccion cronometrada */
    struct timespec t_inicio, t_fin;
    clock_gettime(CLOCK_MONOTONIC, &t_inicio);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int suma = 0;
            for (int k = 0; k < n; k++) {
                /* A se recorre por fila (A[i][k]) y BT tambien por fila (BT[j][k] = B[k][j]):
                   ambos accesos son consecutivos en memoria */
                suma += A[i * n + k] * BT[j * n + k];
            }
            C[i * n + j] = suma;
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t_fin);
    double tiempo_kernel = diferencia_segundos(t_inicio, t_fin);
    /* FIN DEL KERNEL */

    /* Verificacion independiente */
    long long suma_C_directa = 0;
    for (int i = 0; i < n * n; i++) {
        suma_C_directa += C[i];
    }

    long long *suma_cols_A  = (long long *) calloc((size_t) n, sizeof(long long));
    long long *suma_filas_B = (long long *) calloc((size_t) n, sizeof(long long));
    if (suma_cols_A == NULL || suma_filas_B == NULL) {
        free(A); free(BT); free(C); free(suma_cols_A); free(suma_filas_B);
        return 1;
    }

    /* suma_cols_A[k]  = suma de la columna k de A
       suma_filas_B[k] = suma de la fila k de B = suma de la columna k de BT */
    for (int i = 0; i < n; i++) {
        for (int k = 0; k < n; k++) {
            suma_cols_A[k]  += A[i * n + k];
            suma_filas_B[k] += BT[i * n + k];
        }
    }

    long long suma_C_verificacion = 0;
    for (int k = 0; k < n; k++) {
        suma_C_verificacion += suma_cols_A[k] * suma_filas_B[k];
    }

    int verificacion_ok = (suma_C_directa == suma_C_verificacion);

    /* única salida del programa */
    printf("N=%d MAX_VAL=%d tiempo_kernel=%.9f verificacion=%s\n",
           n, MAX_VAL, tiempo_kernel, verificacion_ok ? "OK" : "FALLIDA");

    free(A);
    free(BT);
    free(C);
    free(suma_cols_A);
    free(suma_filas_B);

    return 0;
}