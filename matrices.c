//
// Created by Maxwell Slater on 20/9/2024.
//

#include "matrices.h"
#include "sparsematrix.h"

int** createMatrix(float prob)
{
    int** matrix = calloc(DEFAULT_SIZE, sizeof(int*));
    for (int i = 0; i < DEFAULT_SIZE; i++)
    {
        matrix[i] = (int*) calloc(DEFAULT_SIZE, sizeof(int));
        #pragma omp parallel for
        for (int j = 0; j < DEFAULT_SIZE; j++)
        {
            if( rand() % LIMIT < (int)LIMIT * prob) {
                matrix[i][j] = rand() % LIMIT;
            }
        }
    }
    printf("\t\tAllocated %lu bytes for new matrix...\n", DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));
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


    for (int i = 0; i < DEFAULT_SIZE; i++) {
        for (int j = 0; j < DEFAULT_SIZE; j++) {
            #pragma omp parallel for schedule(dynamic)
            for (int k = 0; k < DEFAULT_SIZE; k++) {
                #pragma omp critical
                result[i][j] += matrix_1[i][k] * matrix_2[k][j];
                printf("%2d",omp_get_thread_num());
            }
            printf("[%d][%d]>",i,j);
        }
    } printf("\n");

    return result;
}