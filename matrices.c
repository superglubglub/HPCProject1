//
// Created by Maxwell Slater on 20/9/2024.
//

#include "matrices.h"
#include "sparsematrix.h"
#include <omp.h>
#include <limits.h>

// Xorshift32 PRNG
uint32_t xorshift32(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

uint8_t* createMatrix(float prob)
{
    uint8_t* matrix = malloc(DEFAULT_SIZE * DEFAULT_SIZE * sizeof(uint8_t));
    printf("\t\tAllocated %lu bytes for new matrix...\n", DEFAULT_SIZE * DEFAULT_SIZE * sizeof(uint8_t));

    const uint32_t threshold = (uint32_t)(prob * UINT_MAX);

    #pragma omp parallel
    {
        uint32_t seed = time(NULL) ^ omp_get_thread_num();
        #pragma omp parallel for schedule(static, BLOCK_SIZE)
        for (int i = 0; i < DEFAULT_SIZE; i++){
            for (int j = 0; j < DEFAULT_SIZE; j++){
                uint32_t rand_val = xorshift32(&seed);
                if(rand_val < threshold) {
                    matrix[i * DEFAULT_SIZE + j] = (rand_val % 9) + 1;
                } else {
                    matrix[i * DEFAULT_SIZE + j] = 0;
                }
            }
        }
    }

    return matrix;
}

void printMatrix(int* matrix) {
    for (int i = 0; i < DEFAULT_SIZE; i++) {
        for (int j = 0; j < DEFAULT_SIZE; j++) {
            printf("[%2d]", matrix[i * DEFAULT_SIZE + j]);
        }
        printf("\n");
    }
    printf("\n\n\n______________\n\n\n");
}

void freeMiniMatrix(uint8_t *matrix) {
    free(matrix);
}

void freeMatrix(int *matrix) {
    free(matrix);
}

int testMatrix(int* matrix_1, int* matrix_2) {
    int failures = 0;
    #pragma omp parallel for
    for (int i = 0; i < DEFAULT_SIZE; i++) {
        for (int j = 0; j < DEFAULT_SIZE; j++) {
            if (matrix_1[i * DEFAULT_SIZE + j] != matrix_2[i * DEFAULT_SIZE + j]) {
                //printf("[%d != %d] FAIL\t", matrix_1[i][j], matrix_2[i][j]);
                failures += 1;
            } else {
                //printf("[%d == %d] PASS\t", matrix_1[i][j], matrix_2[i][j]);
            }
        } //printf("\n");
    }
    return failures;
}

int* multiplyMatrix(uint8_t* matrix_1, uint8_t* matrix_2) {
    printf("\t\tAttempting to multiply matrices...\n");
    int* result = malloc(DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));
    printf("\t\tAllocated %lu bytes for matrix...\n", DEFAULT_SIZE * sizeof(int) * DEFAULT_SIZE);

    int i, j, k, tmp, blksize;
    #pragma omp parallel default(shared
    {
        #pragma omp single
        {
            int num_threads = omp_get_num_threads();
            blksize = (num_threads + 1) / DEFAULT_SIZE;
        }
        #pragma omp for private(i,j,k) schedule(static, blksize) reduction(+:tmp)
        for (i = 0; i < DEFAULT_SIZE; i++)
        {
            for (j = 0; j < DEFAULT_SIZE; j++)
            {
                tmp = 0;
                for (k = 0; k < DEFAULT_SIZE; k++)
                {
                    tmp += matrix_1[i * DEFAULT_SIZE + k] * matrix_2[k * DEFAULT_SIZE + j];
                    //printf("%2d",omp_get_thread_num());
                }
                result[i * DEFAULT_SIZE + j] = tmp;
                //printf("[%d][%d]>",i,j);
            }
        }
    }


    return result;
}