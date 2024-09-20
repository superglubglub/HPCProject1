#include "sparsematrix.h"
#include "matrices.h"

void compressValues(MultiMatrix *matrix)
{
    matrix->values = (SparseRow*) calloc(DEFAULT_SIZE, sizeof(SparseRow));
    matrix->indexes = (SparseRow*) calloc(DEFAULT_SIZE, sizeof(SparseRow));
    for (int i = 0; i < DEFAULT_SIZE; i++) {
        // create the indexes with all the space
        matrix->values[i] = initRow();
        matrix->indexes[i] = initRow();
        for (int j = 0; j < DEFAULT_SIZE; j++) {
            if (matrix->matrix[i][j] != 0) {
                matrix->indexes[i].col[matrix->indexes[i].size - 1] = j;
                matrix->values[i].col[matrix->values[i].size - 1] = matrix->matrix[i][j];
                printf("Added value %d at index [%d][%d]\n",matrix->values[i].col[matrix->values[i].size-1],i,matrix->indexes[i].col[matrix->indexes[i].size-1]);
                matrix->indexes[i].size++; matrix->values[i].size++;
            }
        }
        compressRow(&matrix->values[i]);
        compressRow(&matrix->indexes[i]);
    }
    printf("Compression Complete\n");
}

//naive matrix multiplication algorithm


void freeSim(SIM s) {
    freeMatrix(s.matrix_1.matrix);
    freeMatrix(s.matrix_2.matrix);
    freeMatrix(s.multi_large);
    freeSparseMatrix(s.matrix_1.values);
    freeSparseMatrix(s.matrix_1.indexes);
    freeSparseMatrix(s.matrix_2.values);
    freeSparseMatrix(s.matrix_2.indexes);
    freeMatrix(s.multi_small);
}

int simulate(float prob) {
    SIM simulation = {
        .matrix_1 = {.matrix = createMatrix(prob)},
        .matrix_2 = {.matrix = createMatrix(prob)},
    }; printf("made initial matrices...\n");
    simulation.multi_large = multiplyMatrix(simulation.matrix_1.matrix, simulation.matrix_2.matrix); printf("multiplied matrices...\n");
    printMatrix(simulation.multi_large);
    compressValues(&simulation.matrix_1);
    compressValues(&simulation.matrix_2); printf("compressed matrices...\n");
    simulation.multi_small = multiplySparseMatrices(simulation.matrix_1, simulation.matrix_2); printf("multiplied compression matrices...\n");
    printMatrix(simulation.multi_small);
    int test = testMatrix(simulation.multi_small, simulation.multi_large); printf("tested matrices...\n");
    freeSim(simulation); printf("freed matrices...\n");
    return test;
}

int main(void)
{
    FILE *fp = initLogFile();
    for (int i = 0; i < 3; i++) {
        clock_t start = clock();
        STATS stats = {
            .num_threads = 1,
            .matrix_size = DEFAULT_SIZE,
            .prob = DEFAULT_PROBABILITIES[i],
        };
        int failures = simulate(DEFAULT_PROBABILITIES[i]);
        if(failures > 0) {
            clock_t end = clock();
            stats.runtime = getRuntime(start, end);
            printf("Matrix algorithm failed with %d errors!\n", failures);
            writeFailure(fp, stats);
            return 1;
        }
        clock_t end = clock();
        stats.runtime = getRuntime(start, end);
        writeLogs(fp, stats);
        printf("\tAlgorithm passed at probability %.2f!\t\n", DEFAULT_PROBABILITIES[i]);
    }
    fclose(fp);
    printf("All tests passed!\n");
    return 0;
}

/* Some considerations and notes (algorithms and computation-wise):
 *  - Technically the entire matrix can be considered one long array if we terminate at every %(size) interval
 *  - Each integer should be uint8 to save on space
 *  - Could store compression as an array, but this would waste so much space in the worst case
 *  - Maybe allocated probability% space then realloc using a seperate thread?
 *  - For 8bit integers a 100k*100k matrix uses 10gb space
 *  - Worst case for any probability is 100% space usage, so 30gb space
 *  - But the expected case of 5% means only ~500MB of space used per matrix, or ~1GB for both (10x improvement)
 *  - If we used a linked list then we have an excessive 32 (64?) bytes of space used per integer
 *  - For a doubly linked list using 32-bit addresses, this means we store 8 times the amount of information, or 8gb
 *  - We can binarily partition the reallocation, e.g. we allocate 0.05% space then 0.025% space then 0.0175% and so on
 *  - We would have to realloc log2(n) times maximum (~17 for a 100k length)
 *  - Low-level approach:
 *      - We have our original 1gb sized array
 *      - We want to reduce it to a smaller size
 *  - all arrays should be *restrict pointers
 *  - we can just compress the original array to contain tuples of value, index
 *  - Nevermind it has to be dynamic allocation
 *  - Well, two schools of thought I guess. I can just realloc the array to allow for more numbers.
 */