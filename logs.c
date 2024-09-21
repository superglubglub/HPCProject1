//
// Created by Maxwell Slater on 18/9/2024.
//

#include "sparsematrix.h"

FILE* initLogFile() {
    FILE *fp = fopen("../log.txt", "at+");
    if(!fp) fp = fopen("../log.txt", "wt");
    fprintf(fp, "\n\t// NEW TEST //\t\n");
    return fp;
}

int writeLogs(FILE *fp, STATS stats) {
    fprintf(fp,
        "-----\nMatrix Size: %d\nMatrix Prob: %.6f\nNumber of Threads: %d\n",
        stats.matrix_size, stats.prob, stats.num_threads);
    return 0;
}

int writeFailure(FILE *fp, STATS stats) {
    fprintf(fp,
        "Operation failed in %.6fs\n",
        stats.runtime);
    return 0;
}

int closeLogs(FILE *fp, STATS stats) {
    fprintf(fp,
        "Final Runtime: %.6fs\n",
        stats.runtime);
    fflush(fp);
    return 0;
}

int writeOperation(FILE *fp, char* funcName, STATS* stats) {
    double operationTime = getRuntime(stats->start_time, omp_get_wtime());
    fprintf(fp,
        "\tPerformed %s in %fs\n",
        funcName,getRuntime(stats->runtime, operationTime));
    fflush(fp);
    stats->runtime = operationTime;
    return 0;
}

double getRuntime(double start, double end) {
    return end - start;
}



