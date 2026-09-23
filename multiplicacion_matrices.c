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
    int *A = (int *) malloc((size_t) n * (size_t) n * sizeof(int));
    int *B = (int *) malloc((size_t) n * (size_t) n * sizeof(int));
    int *C = (int *) malloc((size_t) n * (size_t) n * sizeof(int));


    if (A == NULL || B == NULL || C == NULL) {
        free(A); free(B); free(C);
        return 1;
    }


    /* Llenado con enteros positivos aleatorios en [1, MAX_VAL] */
    for (int i = 0; i < n * n; i++) {
        A[i] = 1 + rand() % MAX_VAL;
        B[i] = 1 + rand() % MAX_VAL;
        C[i] = 0;
    }


    /*KERNEL: unica seccion cronometrada */
    struct timespec t_inicio, t_fin;
    clock_gettime(CLOCK_MONOTONIC, &t_inicio);


    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int suma = 0;
            for (int k = 0; k < n; k++) {
                suma += A[i * n + k] * B[k * n + j];
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


    long long *suma_filas_A = (long long *) calloc((size_t) n, sizeof(long long));
    long long *suma_cols_B  = (long long *) calloc((size_t) n, sizeof(long long));
    if (suma_filas_A == NULL || suma_cols_B == NULL) {
        free(A); free(B); free(C); free(suma_filas_A); free(suma_cols_B);
        return 1;
    }


    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            suma_filas_A[k] += A[i * n + k];
        }
        for (int j = 0; j < n; j++) {
            suma_cols_B[k] += B[k * n + j];
        }
    }


    long long suma_C_verificacion = 0;
    for (int k = 0; k < n; k++) {
        suma_C_verificacion += suma_filas_A[k] * suma_cols_B[k];
    }


    int verificacion_ok = (suma_C_directa == suma_C_verificacion);


    /* única salida del programa */
    printf("N=%d MAX_VAL=%d tiempo_kernel=%.9f verificacion=%s\n",
           n, MAX_VAL, tiempo_kernel, verificacion_ok ? "OK" : "FALLIDA");


    free(A);
    free(B);
    free(C);
    free(suma_filas_A);
    free(suma_cols_B);


    return 0;
}
