#include "sparsematrix.h"
#include "matrices.h"
#include <unistd.h>
#include <omp.h>

void compressValues(MultiMatrix *matrix)
{
    matrix->values = (SparseRow*) malloc(DEFAULT_SIZE * sizeof(SparseRow));
    matrix->indexes = (SparseRow*) malloc( DEFAULT_SIZE * sizeof(SparseRow));
    for (int i = 0; i < DEFAULT_SIZE; i++) {
        // create the indexes with all the space
        matrix->values[i] = initRow();
        matrix->indexes[i] = initRow();
        #pragma omp parallel for
        for (int j = 0; j < DEFAULT_SIZE; j++) {
            if (matrix->matrix[i * DEFAULT_SIZE + j] != 0) {
                matrix->indexes[i].col[matrix->indexes[i].size - 1] = j;
                matrix->values[i].col[matrix->values[i].size - 1] = matrix->matrix[i * DEFAULT_SIZE + j];
                matrix->indexes[i].size++; matrix->values[i].size++;
            }
        }
        cmpRowMem(&matrix->values[i]);
        cmpRowMem(&matrix->indexes[i]);
    }
}

void freeSim(SIM s) {
    free(s.matrix_1.matrix);
    free(s.matrix_2.matrix);
    free(s.multi_large);
    freeSparseMatrix(s.matrix_1.values);
    freeSparseMatrix(s.matrix_1.indexes);
    freeSparseMatrix(s.matrix_2.values);
    freeSparseMatrix(s.matrix_2.indexes);
    free(s.multi_small);
}

int simulate(float prob, STATS* stats, FILE *fp) {
    SIM simulation = {
        .matrix_1 = {.matrix = createMatrix(prob)},
        .matrix_2 = {.matrix = createMatrix(prob)},
    }; printf("\tMade matrices...\n");
    writeOperation(fp, "M_CREAT:", stats);
    simulation.multi_large = multiplyMatrix(simulation.matrix_1.matrix, simulation.matrix_2.matrix);
    printf("\tMultiplied matrices...\n");
    writeOperation(fp, "M_MULTI:", stats);

    compressValues(&simulation.matrix_1);
    compressValues(&simulation.matrix_2);
    printf("\tCompressed matrices...\n");
    writeOperation(fp, "M_COMPR:", stats);
    simulation.multi_small = multiplySparseMatrices(simulation.matrix_1, simulation.matrix_2);
    printf("\tMultiplied compression matrices...\n");
    writeOperation(fp, "C_MULTI:", stats);
    int test = testMatrix(simulation.multi_small, simulation.multi_large);
    printf("Tested compressed matrix multiplication result...\n");
    writeOperation(fp, "VERIFIC:", stats);

    freeSim(simulation);
    return test;
}

int main(int argc, char **argv)
{
    int iterations = 1;
    int threadcount = NUM_THREADS;
    int opt;
    while((opt = getopt(argc, argv, "i:t:")) != -1) {
        switch(opt) {
            case 'I':
                iterations = atoi(optarg);
                break;
            case 'T':
                threadcount = atoi(optarg);
                break;
            case '?':
            default: /* '?' */
                printf("Usage: %s [-I] number of iterations [-T] max threads\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    //set the number of threads
    omp_set_num_threads(threadcount);
    FILE *fp = initLogFile();
    for (int i = 0; i < 3; i++) {

        double start = omp_get_wtime();
        STATS stats = {
            .num_threads = threadcount,
            .matrix_size = DEFAULT_SIZE,
            .prob = DEFAULT_PROBABILITIES[i],
            .start_time = omp_get_wtime(),
            .runtime = 0,
        };
        printf("Starting simulation %d of 3 with probability %.2f on size %d\n", i+1, DEFAULT_PROBABILITIES[i], stats.matrix_size);
        writeLogs(fp, stats);
        int failures = simulate(DEFAULT_PROBABILITIES[i], &stats, fp);
        double end = omp_get_wtime();
        stats.runtime = getRuntime(start, end);
        if(failures > 0) {
            printf("Matrix algorithm failed with %d errors!\n", failures);
            writeFailure(fp, stats);
        }
        closeLogs(fp, stats);
        printf("Simulation %d completed in %.2f seconds\n", i+1, stats.runtime);
    }
    fclose(fp);
    printf("All tests passed!\n");
    exit(0);
}