#ifndef CMATRIX_H
#define CMATRIX_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
// #include <omp.h>
#include "vars.h"

typedef struct {
    uint8_t* col;
    size_t size;
} SparseRow;

typedef struct {
    int** matrix;
    SparseRow* values;
    SparseRow* indexes;
} MultiMatrix;

typedef struct {
    //known at creation
    int num_threads;
    int matrix_size;
    float prob;
    //not known at creation
    double runtime;
    int memsize;
} STATS;

typedef struct {
    MultiMatrix matrix_1;
    MultiMatrix matrix_2;
    int** multi_large;
    int** multi_small;
} SIM;

//crow shit
SparseRow initRow();
void compressRow(SparseRow* row);
void addSparseValue(SparseRow valueRow, SparseRow indexRow, int nextindex, int value, int index);
void freeSparseMatrix(SparseRow* matrix);
int** multiplySparseMatrices(MultiMatrix A, MultiMatrix B);
FILE* initLogFile();
int writeLogs(FILE *fp, STATS stats);
int writeFailure(FILE *fp, STATS stats);
double getRuntime(clock_t start, clock_t end);

#endif //CMATRIX_H
