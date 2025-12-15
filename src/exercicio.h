#ifndef EXERCICIO_H
#define EXERCICIO_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef enum {
    SEQ,
    PAR
} mode;

typedef enum {
    PIPE,
    SHM
} ipc;

void program(void);
int is_prime(int N);
int count_primes_seq(int N);
int count_primes_par(int N, int P, ipc ip);
int* partition_interval(int interval, int partitions);

#endif 