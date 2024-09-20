#include "sparsematrix.h"

//initialise the crow with calloc'd values
SparseRow initRow() {
    SparseRow row = {
        .col = (uint8_t*) calloc(DEFAULT_SIZE, sizeof(uint8_t)),
        .size = 1,
    };
    return row;
}

void compressRow(SparseRow* row) {
    uint8_t* tmp = realloc(row->col, sizeof(uint8_t) * row->size);
    row->col = tmp;
}

void freeSparseMatrix(SparseRow* matrix) {
    for(int i = 0; i < DEFAULT_SIZE; i++) {
        free(matrix[i].col);
    }
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

int** multiplySparseMatrices(MultiMatrix A, MultiMatrix B) {
    int** result = (int**) calloc(DEFAULT_SIZE, sizeof(int*));
    #pragma omp parallel for
    for(int i=0; i<DEFAULT_SIZE; i++)
    {
        result[i] = (int*) calloc(DEFAULT_SIZE, sizeof(int));
    }

    #pragma omp parallel for schedule(static)
    for(int i = 0; i < DEFAULT_SIZE; i++) {
        SparseRow* a_values = &A.values[i];
        SparseRow* a_indexes = &A.indexes[i];
        for(int k = 0; k < a_values->size; k++){
            int a_index = a_indexes->col[k];
            int a_value = a_values->col[k];
            SparseRow* b_values = &B.values[a_index];
            SparseRow* b_indexes = &B.indexes[a_index];
            for(int j = 0; j < b_values->size; j++) {
                int b_index = b_indexes->col[j];
                int b_value = b_values->col[j];
                #pragma omp atomic
                result[i][b_index] += a_value * b_value;
            }
        }
    }
    return result;
}

/*
 *  Since it is sorted I can use binary search to find the element, as much as I don't want to write a novel binary
 *  searching algorithm...
 */
