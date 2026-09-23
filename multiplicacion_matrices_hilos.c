#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h> /* Hilos */




#define MAX_VAL 100




/* Hilos */
typedef struct {
    int *A;
    int *B;
    int *C;
    int n;
    int fila_inicio;
    int fila_fin;
} DatosHilo;


/* Hilos */
void *multiplicar_parte(void *arg) {
    DatosHilo *datos = (DatosHilo *) arg;


    for (int i = datos->fila_inicio; i < datos->fila_fin; i++) {
        for (int j = 0; j < datos->n; j++) {
            int suma = 0;
            for (int k = 0; k < datos->n; k++) {
                suma += datos->A[i * datos->n + k] * datos->B[k * datos->n + j];
            }
            datos->C[i * datos->n + j] = suma;
        }
    }


    return NULL;
}




static double diferencia_segundos(struct timespec inicio, struct timespec fin) {
    return (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        return 1;
    }


    int n = atoi(argv[1]);

    /* Hilos */
    int num_hilos = atoi(argv[2]);


    if (n <= 0) {
        return 1;
    }


    /* Hilos */
    if (num_hilos <= 0) {
        return 1;
    }


    /* Hilos */
    if (num_hilos > n) {
        num_hilos = n;
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


    /* Hilos */
    pthread_t *hilos = (pthread_t *) malloc((size_t) num_hilos * sizeof(pthread_t));
    DatosHilo *datos = (DatosHilo *) malloc((size_t) num_hilos * sizeof(DatosHilo));


    if (hilos == NULL || datos == NULL) {
        free(A); free(B); free(C);
        free(hilos); free(datos);
        return 1;
    }


    /* Hilos */
    int filas_por_hilo = n / num_hilos;
    int filas_extra = n % num_hilos;
    int fila_actual = 0;


    for (int i = 0; i < num_hilos; i++) {
        int cantidad_filas = filas_por_hilo;


        if (i < filas_extra) {
            cantidad_filas++;
        }


        datos[i].A = A;
        datos[i].B = B;
        datos[i].C = C;
        datos[i].n = n;
        datos[i].fila_inicio = fila_actual;
        datos[i].fila_fin = fila_actual + cantidad_filas;


        /* Creacion de hilos */
        pthread_create(&hilos[i], NULL, multiplicar_parte, &datos[i]);


        fila_actual += cantidad_filas;
    }


    /*KERNEL: unica seccion cronometrada */
    struct timespec t_inicio, t_fin;
    clock_gettime(CLOCK_MONOTONIC, &t_inicio);


    /* Hilos */
    for (int i = 0; i < num_hilos; i++) {
        pthread_join(hilos[i], NULL);
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
        free(A); free(B); free(C); free(hilos); free(datos);
        free(suma_filas_A); free(suma_cols_B);
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
    printf("N=%d MAX_VAL=%d hilos=%d tiempo_kernel=%.9f verificacion=%s\n",
           n, MAX_VAL, num_hilos, tiempo_kernel, verificacion_ok ? "OK" : "FALLIDA");


    free(A);
    free(B);
    free(C);
    free(hilos);
    free(datos);
    free(suma_filas_A);
    free(suma_cols_B);


    return 0;
}
