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
    uint8_t* matrix = (uint8_t*) calloc((long) size * size * sizeof(uint8_t));
    printf("\t\tAllocated %lu bytes for new matrix...\n", (long) size * size * sizeof(uint8_t));

    const uint32_t threshold = (uint32_t)(prob * UINT_MAX);

    #pragma omp parallel
    {
        uint32_t seed = time(NULL) ^ omp_get_thread_num();
        #pragma omp for schedule(static)
        for (int i = 0; i < size; i++){
            for (int j = 0; j < size; j++){
                if(xorshift32(&seed) < threshold) {
                    matrix[i * size + j] = (xorshift32(&seed) % 9) + 1;
                }
            }
        }
    }

    return matrix;
}

void printMatrix(int* matrix) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            printf("[%2d]", matrix[i * size + j]);
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
    #pragma omp parallel for schedule(static)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (matrix_1[i * size + j] != matrix_2[i * size + j]) {
                //printf("[%d != %d] FAIL\t", matrix_1[i][j], matrix_2[i][j]);
                failures += 1;
            } else {
                //printf("[%d == %d] PASS\t", matrix_1[i][j], matrix_2[i][j]);
            }
        } //printf("\n");
    }
    return failures;
}

void transposeMatrix(uint8_t* matrix, uint8_t* transpose) {
    int i, j;
    for(i = 0; i < size; i++) {
        for(j = 0; j < size; j++) {
            transpose[j * size + i] = matrix[i * size + j];
        }
    }
    return;
}

uint32_t* multiplyMatrix(uint8_t* matrix_1, uint8_t* matrix_2) {
    printf("\t\tAttempting to multiply matrices...\n");
    uint32_t* result = (uint32_t*) malloc((long)(size * size) * sizeof(uint32_t));
    printf("\t\tAllocated %lu bytes for matrix...\n", size * sizeof(uint32_t) * size);

    //get the transpose of matrix_2
    uint8_t* transpose = (uint8_t*) malloc((long)(size * size) * sizeof(uint8_t));
    printf("\t\tAllocated %lu bytes for new transpose matrix...\n",(long)(size * size) * sizeof(uint8_t));
    transposeMatrix(matrix_2, transpose);

    int tmp;
    #pragma omp parallel
    {
        #pragma omp for reduction(+:tmp) schedule(static)
        for (int i = 0; i < size; i++)
        {
            for (int j = 0; j < size; j++)
            {
                tmp = 0;
                for (int k = 0; k < size; k++)
                {
                    tmp += matrix_1[i * size + k] * transpose[k + j * size];
                    //printf("%2d",omp_get_thread_num());
                }
                result[i * size + j] = tmp;
                //printf("[%d][%d]>",i,j);
            }
        }
    }

    free(transpose);


    return result;
}