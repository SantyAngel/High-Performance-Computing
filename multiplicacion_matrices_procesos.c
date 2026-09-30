#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>    /* Procesos: fork */
#include <sys/wait.h>  /* Procesos: waitpid */
#include <sys/mman.h>  /* Procesos: memoria compartida (mmap) */

#define MAX_VAL 100

static double diferencia_segundos(struct timespec inicio, struct timespec fin) {
    return (fin.tv_sec - inicio.tv_sec) + (fin.tv_nsec - inicio.tv_nsec) / 1e9;
}

/* Procesos: reserva de memoria compartida entre padre e hijos */
static int *reservar_compartida(size_t n_elementos) {
    void *p = mmap(NULL, n_elementos * sizeof(int),
                   PROT_READ | PROT_WRITE,
                   MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    return (p == MAP_FAILED) ? NULL : (int *) p;
}

/* Procesos: cada proceso hijo calcula un bloque de filas de C */
static void multiplicar_parte(const int *A, const int *B, int *C,
                              int n, int fila_inicio, int fila_fin) {
    for (int i = fila_inicio; i < fila_fin; i++) {
        for (int j = 0; j < n; j++) {
            int suma = 0;
            for (int k = 0; k < n; k++) {
                suma += A[i * n + k] * B[k * n + j];
            }
            C[i * n + j] = suma;
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        return 1;
    }

    int n = atoi(argv[1]);

    /* Procesos */
    int num_procesos = atoi(argv[2]);

    if (n <= 0) {
        return 1;
    }

    /* Procesos */
    if (num_procesos <= 0) {
        return 1;
    }

    /* Procesos */
    if (num_procesos > n) {
        num_procesos = n;
    }

    srand((unsigned int) time(NULL));

    /* Reserva de memoria compartida (mmap en vez de malloc) */
    size_t total = (size_t) n * (size_t) n;
    int *A = reservar_compartida(total);
    int *B = reservar_compartida(total);
    int *C = reservar_compartida(total);

    if (A == NULL || B == NULL || C == NULL) {
        if (A) munmap(A, total * sizeof(int));
        if (B) munmap(B, total * sizeof(int));
        if (C) munmap(C, total * sizeof(int));
        return 1;
    }

    /* Llenado con enteros positivos aleatorios en [1, MAX_VAL] */
    for (size_t i = 0; i < total; i++) {
        A[i] = 1 + rand() % MAX_VAL;
        B[i] = 1 + rand() % MAX_VAL;
        C[i] = 0;
    }

    /* Procesos */
    pid_t *pids = (pid_t *) malloc((size_t) num_procesos * sizeof(pid_t));

    if (pids == NULL) {
        munmap(A, total * sizeof(int));
        munmap(B, total * sizeof(int));
        munmap(C, total * sizeof(int));
        return 1;
    }

    /* Procesos */
    int filas_por_proceso = n / num_procesos;
    int filas_extra = n % num_procesos;
    int fila_actual = 0;

    /*KERNEL: unica seccion cronometrada (incluye creacion y espera de procesos) */
    struct timespec t_inicio, t_fin;
    clock_gettime(CLOCK_MONOTONIC, &t_inicio);

    int error_proceso = 0;

    for (int i = 0; i < num_procesos; i++) {
        int cantidad_filas = filas_por_proceso;

        if (i < filas_extra) {
            cantidad_filas++;
        }

        int fila_inicio = fila_actual;
        int fila_fin = fila_actual + cantidad_filas;

        /* Creacion de procesos */
        pid_t pid = fork();

        if (pid < 0) {
            pids[i] = -1;
            error_proceso = 1;
        } else if (pid == 0) {
            /* Proceso hijo: calcula su bloque de filas y termina */
            multiplicar_parte(A, B, C, n, fila_inicio, fila_fin);
            _exit(0);
        } else {
            pids[i] = pid;
        }

        fila_actual += cantidad_filas;
    }

    /* Procesos: el padre espera a todos los hijos */
    for (int i = 0; i < num_procesos; i++) {
        if (pids[i] > 0) {
            int estado = 0;
            waitpid(pids[i], &estado, 0);
            if (!WIFEXITED(estado) || WEXITSTATUS(estado) != 0) {
                error_proceso = 1;
            }
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &t_fin);
    double tiempo_kernel = diferencia_segundos(t_inicio, t_fin);
    /* FIN DEL KERNEL */

    /* Verificacion independiente */
    long long suma_C_directa = 0;
    for (size_t i = 0; i < total; i++) {
        suma_C_directa += C[i];
    }

    long long *suma_filas_A = (long long *) calloc((size_t) n, sizeof(long long));
    long long *suma_cols_B  = (long long *) calloc((size_t) n, sizeof(long long));
    if (suma_filas_A == NULL || suma_cols_B == NULL) {
        munmap(A, total * sizeof(int));
        munmap(B, total * sizeof(int));
        munmap(C, total * sizeof(int));
        free(pids);
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

    int verificacion_ok = (suma_C_directa == suma_C_verificacion) && !error_proceso;

    /* única salida del programa */
    printf("N=%d MAX_VAL=%d procesos=%d tiempo_kernel=%.9f verificacion=%s\n",
           n, MAX_VAL, num_procesos, tiempo_kernel, verificacion_ok ? "OK" : "FALLIDA");

    munmap(A, total * sizeof(int));
    munmap(B, total * sizeof(int));
    munmap(C, total * sizeof(int));
    free(pids);
    free(suma_filas_A);
    free(suma_cols_B);

    return 0;
}
