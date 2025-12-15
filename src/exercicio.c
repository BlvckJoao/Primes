#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <stdint.h>

#include <time.h>
#include <string.h>

#include "exercicio.h"

int* partition_interval(int interval, int partitions) {
    if (partitions <= 0 || interval < 0) return NULL;

    int *bounds = malloc((partitions + 1) * sizeof(int));
    if (!bounds) return NULL;

    int base = interval / partitions;
    int rem  = interval % partitions;

    bounds[0] = 0;
    for (int i = 1; i <= partitions; ++i) {
        int add = base + ((i - 1) < rem ? 1 : 0);
        bounds[i] = bounds[i - 1] + add;
    }

    return bounds;
}

int is_prime(int n) {
    if (n < 2) return 0;
    if (n == 2) return 1;
    if (n % 2 == 0) return 0;

    for (int i = 3; (int64_t)i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

int count_primes_in_range(int start, int end) {
    int cnt = 0;
    for (int i = start; i <= end; ++i) {
        if (is_prime(i)) cnt++;
    }
    return cnt;
}

int count_primes_seq(int N) {
    if (N < 2) return 0;

    int cnt = 1;
    for (int i = 3; i <= N; i += 2) {
        if (is_prime(i)) cnt++;
    }
    return cnt;
}

int count_primes_par(int N, int P, ipc ip) {
    if (N < 2) return 0;

    int workers = P;
    if (workers <= 0) {
        workers = (int)sysconf(_SC_NPROCESSORS_ONLN);
        if (workers <= 0) workers = 1;
    }

    int total_items = N - 1;
    if (workers > total_items) workers = total_items;

    int *bounds = partition_interval(total_items, workers);
    if (!bounds) return -1;

    pid_t *pids = calloc(workers, sizeof(pid_t));
    if (!pids) {
        free(bounds);
        return -1;
    }

    int result = 0;

    if (ip == SHM) {
        int *shm = mmap(NULL, workers * sizeof(int),
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        if (shm == MAP_FAILED) {
            free(bounds);
            free(pids);
            return -1;
        }

        for (int i = 0; i < workers; ++i) {
            pid_t pid = fork();
            if (pid == 0) {
                int start = 2 + bounds[i];
                int end   = 2 + bounds[i + 1] - 1;
                shm[i] = count_primes_in_range(start, end);
                _exit(0);
            }
            pids[i] = pid;
        }

        for (int i = 0; i < workers; ++i) {
            waitpid(pids[i], NULL, 0);
            result += shm[i];
        }

        munmap(shm, workers * sizeof(int));
    }
    else {
        int (*pipes)[2] = malloc(workers * sizeof(int[2]));
        if (!pipes) {
            free(bounds);
            free(pids);
            return -1;
        }

        for (int i = 0; i < workers; ++i) {
            if (pipe(pipes[i]) == -1) {
                free(bounds);
                free(pids);
                free(pipes);
                return -1;
            }

            pid_t pid = fork();
            if (pid == 0) {
                for (int j = 0; j < workers; ++j) {
                    if (j != i) {
                        close(pipes[j][0]);
                        close(pipes[j][1]);
                    }
                }
                close(pipes[i][0]);

                int start = 2 + bounds[i];
                int end   = 2 + bounds[i + 1] - 1;
                int cnt   = count_primes_in_range(start, end);

                if (write(pipes[i][1], &cnt, sizeof(int)) == -1) {
                    close(pipes[i][1]);
                    _exit(1);
                }
                close(pipes[i][1]);
                _exit(0);
            }

            pids[i] = pid;
            close(pipes[i][1]);
        }

        for (int i = 0; i < workers; ++i) {
            int cnt = 0;
            read(pipes[i][0], &cnt, sizeof(int));
            close(pipes[i][0]);
            waitpid(pids[i], NULL, 0);
            result += cnt;
        }

        free(pipes);
    }

    free(bounds);
    free(pids);
    return result;
}

static double diff_ms(struct timespec a, struct timespec b) {
    return (b.tv_sec - a.tv_sec) * 1000.0 +
           (b.tv_nsec - a.tv_nsec) / 1e6;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        fprintf(stderr,
            "Uso:\n"
            "  %s seq N\n"
            "  %s par N P pipe|shm\n",
            argv[0], argv[0]);
        return 1;
    }

    struct timespec t0, t1;
    int N = atoi(argv[2]);
    int primes = 0;

    if (strcmp(argv[1], "seq") == 0) {
        clock_gettime(CLOCK_MONOTONIC, &t0);
        primes = count_primes_seq(N);
        clock_gettime(CLOCK_MONOTONIC, &t1);

        printf("mode=seq N=%d primes=%d time_ms=%.3f\n",
               N, primes, diff_ms(t0, t1));
    }

    else if (strcmp(argv[1], "par") == 0) {
        if (argc < 5) {
            fprintf(stderr,
                "Uso: %s par N P pipe|shm\n", argv[0]);
            return 1;
        }

        int P = atoi(argv[3]);
        ipc ipc_mode;

        if (strcmp(argv[4], "pipe") == 0)
            ipc_mode = PIPE;
        else if (strcmp(argv[4], "shm") == 0)
            ipc_mode = SHM;
        else {
            fprintf(stderr, "IPC inválido: %s\n", argv[4]);
            return 1;
        }

        clock_gettime(CLOCK_MONOTONIC, &t0);
        primes = count_primes_par(N, P, ipc_mode);
        clock_gettime(CLOCK_MONOTONIC, &t1);

        printf("mode=par N=%d P=%d ipc=%s primes=%d time_ms=%.3f\n",
               N, P, argv[4], primes, diff_ms(t0, t1));
    }

    else {
        fprintf(stderr, "Modo inválido: %s\n", argv[1]);
        return 1;
    }

    return 0;
}