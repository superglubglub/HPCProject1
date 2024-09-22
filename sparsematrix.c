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

// special initialiser for contiguous memory access
SparseRow initContigRow() {
    SparseRow row = {
        .col = (int*) malloc(DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int)),
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
void transposeSparseMatrix(SparseRow* sparseValues, SparseRow* sparseIndexes, SparseRow transposeValues, SparseRow transposeIndexes) {
    int i, j;
    for(i = 0; i < DEFAULT_SIZE; i++) {
        for(j = 0; j < sparseIndexes[i].size; j++) {
            transposeValues.col[transposeValues.size - 1] = sparseValues[j].col[i];
            transposeIndexes.col[transposeValues.size - 1] = sparseIndexes[j].col[i];
            transposeValues.size++; transposeIndexes.size++;
        }
    }
    cmpRowMem(&transposeIndexes);
    cmpRowMem(&transposeValues);
}

int* multiplySparseMatrices(MultiMatrix A, MultiMatrix B) {
    int* result = malloc(DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));
    printf("\t\tAllocated %lu bytes for sparse multiplication...\n", DEFAULT_SIZE * DEFAULT_SIZE * sizeof(int));

    SparseRow transposeValues = initContigRow();
    SparseRow transposeIndexes = initContigRow();
    transposeSparseMatrix(
        B.values, B.indexes,
        transposeValues, transposeIndexes
    );
    printf("\t\tAllocated %lu bytes for tranposed sparse indexes...\n", transposeIndexes.size * sizeof(int));
    printf("\t\tAllocated %lu bytes for tranposed sparse values...\n", transposeValues.size * sizeof(int));

    int tmp;
    int nextIndex = 0;
    int lastIndex = 0;
    #pragma omp parallel for schedule(dynamic, 1) reduction(+:tmp)
    for (int i = 0; i < DEFAULT_SIZE; i++)
    {
        SparseRow* a_values = &A.values[i];
        SparseRow* a_indexes = &A.indexes[i];
        for (int j = 0; j < transposeIndexes.size; j++)
        {
            tmp = 0;
            for(int k = 0; k < a_indexes->size; nextIndex++)
            {
                int transposeIndex = transposeIndexes.col[nextIndex];
                //if we finished the row
                if(lastIndex >= transposeIndex)
                {
                    lastIndex = transposeIndex;
                    break;
                }
                //if we match an index
                if(a_indexes->col[k] == transposeIndex)
                {
                    tmp += a_values->col[k] * transposeValues.col[nextIndex];
                }

                //if the a_index is less, then increment k
                if(a_indexes->col[k] < transposeIndex)
                {
                    k++;
                }
            }
            result[i * DEFAULT_SIZE + j] = tmp;
        }
    }

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
