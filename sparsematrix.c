#include "sparsematrix.h"
#include <omp.h>

//initialise the crow with calloc'd values
SparseRow initRow() {
    SparseRow row = {
        .col = (int*) calloc(DEFAULT_SIZE, sizeof(int)),
        .size = 1,
    };
    return row;
}

void cmpContigSparse(SparseMatrix sparseMatrix) {
    uint8_t *tmp_val = realloc(sparseMatrix.val, sizeof(uint8_t) * sparseMatrix.offset[DEFAULT_SIZE - 1]);
    int *tmp_idx = realloc(sparseMatrix.idx, sizeof(int) * sparseMatrix.offset[DEFAULT_SIZE - 1]);
    sparseMatrix.val = tmp_val;
    sparseMatrix.idx = tmp_idx;
}

void cmpRowMem(SparseRow* row) {
    int* tmp = realloc(row->col, sizeof(int) * row->size);
    row->col = tmp;
}

void freeSparseMatrix(SparseMatrix* sparseMatrix) {
    free(sparseMatrix->idx);
    free(sparseMatrix->val);
    free(sparseMatrix->offset);
}

/* void freeSparseMatrix(SparseRow* sparseMatrix) {
    for(int i = 0; i < DEFAULT_SIZE; i++) {
        free(sparseMatrix[i].col);
    } free(sparseMatrix);
} */

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

void transposeSparse(SparseMatrix sparse, uint8_t *transpose) {
    int i, j, lim;
    for(i = 0; i < DEFAULT_SIZE; i++) {
        if(i != 0) j = sparse.offset[i - 1];
        else j = 0;
        for(j = sparse.offset[i] - j; j < sparse.offset[i]; j++) {
            transpose[sparse.idx[j] * DEFAULT_SIZE + i] = sparse.val[j];
        }
    }
}

int* multiplySparseMatrices(MultiMatrix A, MultiMatrix B) {
    int* result = malloc(DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));
    printf("\t\tAllocated %lu bytes for sparse multiplication...\n", DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));

    uint8_t* transpose = calloc(DEFAULT_SIZE * DEFAULT_SIZE, sizeof(uint8_t));
    transposeSparse(B.sparse, transpose);

    int i, j, k, tmp;
    #pragma omp parallel for reduction(+:tmp) private(i, j, k) shared(result) schedule(dynamic, 1)
    for(i = 0; i < DEFAULT_SIZE; i++) {
        for(j = 0; j < DEFAULT_SIZE; j++) {
            tmp = 0;
            if(i != 0) k = A.sparse.offset[i - 1];
            else k = 0;
            for(k = A.sparse.offset[i] - k; k < A.sparse.offset[i]; k++) {
                tmp += (A.sparse.val[k] * transpose[j * DEFAULT_SIZE + A.sparse.idx[k]]);
            }
            result[i * DEFAULT_SIZE + j] = tmp;
        }
    }

    free(transpose);
    return result;
}

/* int* multiplySparseMatrices(MultiMatrix A, MultiMatrix B) {
    int* result = malloc(DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));
    printf("\t\tAllocated %lu bytes for sparse multiplication...\n", DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));

    int tmp;
    #pragma omp parallel for schedule(static, BLOCK_SIZE) reduction (+:tmp)
    for(int i = 0; i < DEFAULT_SIZE; i++) {
        SparseRow* a_values = &A.values[i];
        SparseRow* a_indexes = &A.indexes[i];
        for(int j = 0; j < DEFAULT_SIZE; j++){
            tmp = 0;
            for(int k = 0; k < a_values->size; k++) {
                int a_index = a_indexes->col[k]; int a_value = a_values->col[k];
                SparseRow* b_values = &B.values[a_index]; SparseRow* b_indexes = &B.indexes[a_index];
                int b_value = findIndex(j, b_indexes, b_values);
                tmp += (a_value * b_value);
            }
            result[i * DEFAULT_SIZE + j] = tmp;
        }
    }

    return result;
}*/

/*
 *  Since it is sorted I can use binary search to find the element, as much as I don't want to write a novel binary
 *  searching algorithm...
 */
