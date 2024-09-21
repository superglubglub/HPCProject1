#include "sparsematrix.h"
#include <omp.h>

//initialise the crow with calloc'd values
SparseRow initRow() {
    SparseRow row = {
        .col = (int*) malloc(DEFAULT_SIZE * sizeof(int)),
        .size = 1,
    };
    return row;
}

void compressRow(SparseRow* row) {
    int* tmp = realloc(row->col, sizeof(int) * row->size);
    row->col = tmp;
}

void freeSparseMatrix(SparseRow* sparseMatrix) {
    for(int i = 0; i < DEFAULT_SIZE; i++) {
        free(sparseMatrix[i].col);
    } free(sparseMatrix);
}

int findIndex(int index, SparseRow* indexes, SparseRow* values) {
    int low = 0;
    int high = indexes->size - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int mid_index = indexes->col[mid];
        if(index == mid_index)
            return values->col[mid];
        if(index > mid_index) {
            low = mid + 1;
        }
        if(index < mid_index) {
            high = mid - 1;
        }
    }
    return 0;
}

int* multiplySparseMatrices(MultiMatrix A, MultiMatrix B) {
    int* result = malloc(DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));
    printf("\t\tAllocated %lu bytes for sparse multiplication...\n", sizeof(result));

    int i, j, k, tmp;
    #pragma omp parallel
    {
        #pragma omp parallel for private(i, j, k) reduction(+:tmp) schedule(static, BLOCK_SIZE)
        for(i = 0; i < DEFAULT_SIZE; i++) {
            SparseRow* a_values = &A.values[i];
            SparseRow* a_indexes = &A.indexes[i];
            for(j = 0; j < DEFAULT_SIZE; j++){
                tmp = 0;
                for(k = 0; k < a_values->size; k++) {
                    int a_value = a_values->col[k]; int a_index = a_indexes->col[k];
                    int b_value = findIndex(a_indexes->col[k], &B.indexes[a_index], &B.values[a_index]);
                    tmp += a_value * b_value;
                }
                result[i * DEFAULT_SIZE + j] += tmp;
            }
        }
    }
    return result;
}

/*
 *  Since it is sorted I can use binary search to find the element, as much as I don't want to write a novel binary
 *  searching algorithm...
 */
