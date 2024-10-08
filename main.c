#include "sparsematrix.h"
#include "matrices.h"
#include "vars.h"
#include <unistd.h>
#include <omp.h>

long size;
char* logfile;
int method;

void compressValues(MultiMatrix *matrix)
{
    matrix->values = (SparseRow*) malloc(size * sizeof(SparseRow));
    matrix->indexes = (SparseRow*) malloc(size * sizeof(SparseRow));
    for (int i = 0; i < size; i++) {
        // create the indexes with all the space
        matrix->values[i] = initRow();
        matrix->indexes[i] = initRow();
        for (int j = 0; j < size; j++) {
            if (matrix->matrix[i * size + j] != 0) {
                matrix->indexes[i].col[matrix->indexes[i].size - 1] = j;
                matrix->values[i].col[matrix->values[i].size - 1] = matrix->matrix[i * size + j];
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

    //matrix generation
    SIM simulation = {
        .matrix_1 = {.matrix = createMatrix(prob)},
        .matrix_2 = {.matrix = createMatrix(prob)},
    }; printf("\tMade matrices...\n");
    writeOperation(fp, "M_CREAT:", stats);

    //matrix multiplication
    simulation.multi_large = multiplyMatrix(simulation.matrix_1.matrix, simulation.matrix_2.matrix);
    printf("\tMultiplied matrices...\n");
    writeOperation(fp, "M_MULTI:", stats);

    //matrix compression
    compressValues(&simulation.matrix_1);
    compressValues(&simulation.matrix_2);
    printf("\tCompressed matrices...\n");
    writeOperation(fp, "M_COMPR:", stats);
    writeSparseMatrixToFile(simulation.matrix_1.values, simulation.matrix_1.indexes);

    //compressed multiplication
    switch(method) {
        case 1:
            simulation.multi_small = multiplySparseMatricesDYNAMIC(simulation.matrix_1, simulation.matrix_2);
            writeOperation(fp, "C_MULTI_DYNA:", stats);
        case 2:
            simulation.multi_small = multiplySparseMatricesAUTO(simulation.matrix_1, simulation.matrix_2);
            writeOperation(fp, "C_MULTI_AUTO:", stats);
            break;
        case 3:
            simulation.multi_small = multiplySparseMatricesGUIDED(simulation.matrix_1, simulation.matrix_2);
            writeOperation(fp, "C_MULTI_GUID:", stats);
            break;
        case 0:
        default:
            simulation.multi_small = multiplySparseMatrices(simulation.matrix_1, simulation.matrix_2);
            writeOperation(fp, "C_MULTI_STAT:", stats);
            break;
    }
    simulation.multi_small = multiplySparseMatrices(simulation.matrix_1, simulation.matrix_2);
    printf("\tMultiplied compression matrices...\n");
    writeOperation(fp, "C_MULTI:", stats);

    //matrix verification
    int test = testMatrix(simulation.multi_small, simulation.multi_large);
    printf("Tested compressed matrix multiplication result...\n");
    writeOperation(fp, "VERIFIC:", stats);

    //matrix emancipation
    freeSim(simulation);
    return test;
}

int main(int argc, char **argv)
{
    size = DEFAULT_SIZE;
    logfile = "logging.txt";
    method = 0;
    int threadcount = NUM_THREADS;
    int opt;
    while((opt = getopt(argc, argv, "s:t:l:b:")) != -1) {
        switch(opt) {
            case 's':
                size = atoi(optarg);
                break;
            case 't':
                threadcount = atoi(optarg);
                break;
            case 'l':
                logfile = optarg;
                break;
            case 'b':
                method = atoi(optarg);
                break;
            case '?':
            default: /* '?' */
                printf("Usage: %s [-i] number of iterations [-t] max threads [-l] logging path\n", argv[0]);
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
            .matrix_size = size,
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