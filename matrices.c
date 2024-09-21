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

int** createMatrix(float prob)
{
    uint8_t** matrix = calloc(DEFAULT_SIZE, sizeof(uint8_t*));
    for (int i = 0; i < DEFAULT_SIZE; i++){
        matrix[i] = (uint8_t*) calloc(DEFAULT_SIZE, sizeof(uint8_t));
    } printf("\t\tAllocated %lu bytes for new matrix...\n", DEFAULT_SIZE * DEFAULT_SIZE * sizeof(uint8_t));

    const uint32_t threshold = (uint32_t)(prob * UINT_MAX);

    #pragma omp parallel
    {
        uint32_t seed = time(NULL) ^ omp_get_thread_num();
        #pragma omp parallel for schedule(dynamic, 1000)
        for (int i = 0; i < DEFAULT_SIZE; i++){
            for (int j = 0; j < DEFAULT_SIZE; j++){
                uint32_t rand_val = xorshift32(&seed);
                if(rand_val < threshold) {
                    matrix[i][j] = (rand_val % 9) + 1;
                }
            }
        }
    }

    return matrix;
}

void printMatrix(int** matrix) {
    for (int i = 0; i < DEFAULT_SIZE; i++) {
        for (int j = 0; j < DEFAULT_SIZE; j++) {
            printf("[%2d]", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n\n\n______________\n\n\n");
}

void freeMatrix(int **matrix) {
    #pragma omp parallel for
    for (int i = 0; i < DEFAULT_SIZE; i++) {
        free(matrix[i]);
    } free(matrix);
}

int testMatrix(int** matrix_1, int** matrix_2) {
    int failures = 0;
    #pragma omp parallel for
    for (int i = 0; i < DEFAULT_SIZE; i++) {
        for (int j = 0; j < DEFAULT_SIZE; j++) {
            if (matrix_1[i][j] != matrix_2[i][j]) {
                //printf("[%d != %d] FAIL\t", matrix_1[i][j], matrix_2[i][j]);
                failures += 1;
            } else {
                //printf("[%d == %d] PASS\t", matrix_1[i][j], matrix_2[i][j]);
            }
        } //printf("\n");
    }
    return failures;
}

int** multiplyMatrix(int **matrix_1, int **matrix_2) {
    printf("\t\tAttempting to multiply matrices...\n");
    int** result = calloc(DEFAULT_SIZE, sizeof(int*));
    for (int i = 0; i < DEFAULT_SIZE; i++) {
        result[i] = calloc(DEFAULT_SIZE, sizeof(int));
    } printf("\t\tAllocated %lu bytes for matrix...\n", DEFAULT_SIZE * sizeof(int) * DEFAULT_SIZE);

    int i, j, k, tmp;
    #pragma omp parallel
    {
        // Create a buffer for blocking
        unsigned int *tmp = (unsigned int *)calloc(BLOCK_SIZE * BLOCK_SIZE, sizeof(unsigned int));

        #pragma omp schedule(dynamic, 1)
        for (i = 0; i < DEFAULT_SIZE; i += BLOCK_SIZE) {
            for (j = 0; j < DEFAULT_SIZE; j += BLOCK_SIZE) {

                // Clear temporary buffer
                for (int x = 0; x < BLOCK_SIZE * BLOCK_SIZE; x++) {
                    tmp[x] = 0;
                }

                for (k = 0; k < DEFAULT_SIZE; k++) {
                    int max_i = (i + BLOCK_SIZE < DEFAULT_SIZE) ? i + BLOCK_SIZE : DEFAULT_SIZE;
                    int max_j = (j + BLOCK_SIZE < DEFAULT_SIZE) ? j + BLOCK_SIZE : DEFAULT_SIZE;
                    int max_k = (k + BLOCK_SIZE < DEFAULT_SIZE) ? k + BLOCK_SIZE : DEFAULT_SIZE;

                    for (int ii = i; ii < max_i; ii++) {
                        for (int jj = j; jj < max_j; jj++) {
                            unsigned int sum = 0;
                            for (int kk = k; kk < max_k; kk++) {
                                sum += matrix_1[ii][kk] * matrix_2[kk][jj];
                            }
                            tmp[(ii-i) * BLOCK_SIZE + (jj-j)] += sum;
                        }
                    }

                    tmp += matrix_1[i][k] * matrix_2[k][j];
                    //printf("%2d",omp_get_thread_num());
                }

                // Update global result
                for (int ii = 0; ii < BLOCK_SIZE && i + ii < DEFAULT_SIZE; ii++) {
                    for (int jj = 0; jj < BLOCK_SIZE && j + jj < DEFAULT_SIZE; jj++) {
                        result[(i+ii)][(j+jj)] = tmp[ii * BLOCK_SIZE + jj];
                    }
                }
                //printf("[%d][%d]>",i,j);
            }
        }

        free(tmp);
    }


    return result;
}