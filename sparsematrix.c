#include "sparsematrix.h"
#include <omp.h>

//initialise the crow with calloc'd values
SparseRow initRow() {
    SparseRow row = {
        .col = (int*) malloc(size * sizeof(int)),
        .size = 1,
    };
    return row;
}

void cmpRowMem(SparseRow* row) {
    int* tmp = realloc(row->col, sizeof(int) * row->size);
    row->col = tmp;
}

void freeSparseMatrix(SparseRow* sparseMatrix) {
    for(int i = 0; i < size; i++) {
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

uint32_t* multiplySparseMatrices(MultiMatrix A, MultiMatrix B) {
    uint32_t* result = (uint32_t*) malloc((long) size * size * sizeof(int));
    printf("\t\tAllocated %lu bytes for sparse multiplication...\n", size * size * sizeof(int));

    int tmp;
    #pragma omp parallel for collapse(2) schedule(static, BLOCK_SIZE)
    for(int i = 0; i < size; i++) {
        for(int j = 0; j < size; j++){
            tmp = 0;
            for(int k = 0; k < A.values[i].size; k++) {
                int a_index = A.indexes[i].col[k]; int a_value = A.values[i].col[k];
                SparseRow* b_values = &B.values[a_index]; SparseRow* b_indexes = &B.indexes[a_index];
                int b_value = findIndex(j, b_indexes, b_values);
                tmp += (a_value * b_value);
            }
            result[i * size + j] = tmp;
        }
    }

    return result;
}

/*
 *  Since it is sorted I can use binary search to find the element, as much as I don't want to write a novel binary
 *  searching algorithm...
 */
