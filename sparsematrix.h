#ifndef CMATRIX_H
#define CMATRIX_H
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <omp.h>
#include "vars.h"

typedef struct {
    int* col;
    size_t size;
} SparseRow;

typedef struct {
    uint8_t* matrix;
    SparseRow* values;
    SparseRow* indexes;
} MultiMatrix;

typedef struct {
    //known at creation
    int num_threads;
    int matrix_size;
    float prob;
    double start_time;
    //not known at creation
    double runtime;
} STATS;

typedef struct {
    MultiMatrix matrix_1;
    MultiMatrix matrix_2;
    int* multi_large;
    int* multi_small;
} SIM;

//crow shit
SparseRow initRow();
void cmpRowMem(SparseRow* row);
void addSparseValue(SparseRow valueRow, SparseRow indexRow, int nextindex, int value, int index);
void freeSparseMatrix(SparseRow* sparseMatrix);
int* multiplySparseMatrices(MultiMatrix A, MultiMatrix B);
FILE* initLogFile();
int writeLogs(FILE *fp, STATS stats);
int writeOperation(FILE *fp, char* funcName, STATS* stats);
int writeFailure(FILE *fp, STATS stats);
int closeLogs(FILE *fp, STATS stats);
double getRuntime(double start, double end);

#endif //CMATRIX_H
