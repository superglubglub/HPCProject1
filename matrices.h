//
// Created by Maxwell Slater on 20/9/2024.
//

#ifndef MATRICES_H
#define MATRICES_H
#include <stdint.h>

uint8_t** createMatrix(float prob);
void printMatrix(int** matrix);
void freeMatrix(int **matrix);
int testMatrix(int** matrix_1, int** matrix_2);
int** multiplyMatrix(int **matrix_1, int **matrix_2);

#endif //MATRICES_H
