#include "sparsematrix.h"
#include "matrices.h"
#include <omp.h>

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
                    matrix->indexes[i].size++; matrix->values[i].size++;
                }
            }
            compressRow(&matrix->values[i]);
            compressRow(&matrix->indexes[i]);
        }
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
    }; printf("Made matrices...\n");
    simulation.multi_large = multiplyMatrix(simulation.matrix_1.matrix, simulation.matrix_2.matrix);
    printf("Multiplied matrices...\n");

    printf("\n\n-------- MATRIX VALUES --------\n\n");
    printf("\n>> MATRIX 1 >>\n"); printMatrix(simulation.matrix_1.matrix);
    printf("\n>> MATRIX 2 >>\n"); printMatrix(simulation.matrix_2.matrix);
    printf("\n>> MULTIPLIED MATRIX >>\n"); printMatrix(simulation.multi_large);

    compressValues(&simulation.matrix_1);
    compressValues(&simulation.matrix_2);
    simulation.multi_small = multiplySparseMatrices(simulation.matrix_1, simulation.matrix_2);

    printf("\n\n-------- FINAL OUTPUT VALUES --------\n\n");
    printf("\n\n-------- MULTIPLIED COMPRESSION MATRIX --------\n\n"); printMatrix(simulation.multi_small);
    printf("\n\n-------- MATRIX OUTPUT TEST --------\n\n"); int test = testMatrix(simulation.multi_small, simulation.multi_large);

    freeSim(simulation);
    return test;
}

int main(void)
{
    FILE *fp = initLogFile();
    //set the number of threads
    omp_set_num_threads(NUM_THREADS);
    printf("Set the number of threads to %d\n",omp_get_num_threads());
    for (int i = 0; i < 3; i++) {
        double start = omp_get_wtime();
        STATS stats = {
            .num_threads = omp_get_num_threads(),
            .matrix_size = DEFAULT_SIZE,
            .prob = DEFAULT_PROBABILITIES[i],
        };
        printf("Starting simulation %d of 3 with probability %.2f\n", i+1, DEFAULT_PROBABILITIES[i]);
        int failures = simulate(DEFAULT_PROBABILITIES[i]);
        double end = omp_get_wtime();
        if(failures > 0) {
            stats.runtime = getRuntime(start, end);
            printf("Matrix algorithm failed with %d errors!\n", failures);
            writeFailure(fp, stats);
            return 1;
        }
        stats.runtime = getRuntime(start, end);
        writeLogs(fp, stats);
        printf("Simulation %d completed in %.2f seconds\n", i+1, stats.runtime);
    }
    fclose(fp);
    printf("All tests passed!\n");
    exit(0);
}