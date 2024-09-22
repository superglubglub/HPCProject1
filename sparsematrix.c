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

void cmpRowMem(SparseRow* row) {
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

//get the transpose of the sparse matrix as a single sparse row
uint8_t* transposeSparseMatrix(SparseRow* sparseValues, SparseRow* sparseIndexes) {

    // init all rows in the transpose matrix
    uint8_t* transpose = calloc(DEFAULT_SIZE * DEFAULT_SIZE, sizeof(uint8_t));
    for(int i = 0; i < DEFAULT_SIZE; i++) {
        for(int j = 0; j < sparseIndexes[i].size; j++) {
            int row = sparseIndexes[i].col[j];
            transpose[row * DEFAULT_SIZE + i] = sparseValues[i].col[j];
        }
    }
    printf("\t\tAllocated %lu bytes for tranposed sparse matrix...\n", DEFAULT_SIZE * DEFAULT_SIZE * sizeof(uint8_t));
    return transpose;
}

int* multiplySparseMatrices(MultiMatrix A, MultiMatrix B) {
    int* result = malloc(DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));
    printf("\t\tAllocated %lu bytes for sparse multiplication...\n", DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));

    uint8_t* transpose = transposeSparseMatrix(B.values, B.indexes);

    int tmp;
    #pragma omp parallel for schedule(dynamic, 1) reduction(+:tmp)
    for (int i = 0; i < DEFAULT_SIZE; i++)
    {
        SparseRow* a_values = &A.values[i];
        SparseRow* a_indexes = &A.indexes[i];
        for (int j = 0; j < DEFAULT_SIZE; j++)
        {
            tmp = 0;
            for(int k = 0; k < a_indexes->size; k++)
            {
                tmp += a_values->col[k] * transpose[j * DEFAULT_SIZE + a_indexes->col[k]];
            }
            result[i * DEFAULT_SIZE + j] = tmp;
        }
    }
    free(transpose);

    /*
    int i, j, k;
    #pragma omp parallel for private(i, j, k) schedule(static, BLOCK_SIZE)
    for(i = 0; i < DEFAULT_SIZE; i++) {
        SparseRow* a_values = &A.values[i];
        SparseRow* a_indexes = &A.indexes[i];
        for(j = 0; j < DEFAULT_SIZE; j++){
            for(k = 0; k < a_values->size; k++) {
                int a_index = a_indexes->col[k]; int a_value = a_values->col[k];
                SparseRow* b_values = &B.values[a_index]; SparseRow* b_indexes = &B.indexes[a_index];
                int b_value = findIndex(j, b_indexes, b_values);
                #pragma omp atomic
                result[i * DEFAULT_SIZE + j] += (a_value * b_value);
            }
        }
    }*/

    return result;
}

/*
 *  Since it is sorted I can use binary search to find the element, as much as I don't want to write a novel binary
 *  searching algorithm...
 */
