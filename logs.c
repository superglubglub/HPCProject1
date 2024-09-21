//
// Created by Maxwell Slater on 18/9/2024.
//

#include "sparsematrix.h"

FILE* initLogFile() {
    FILE *fp, *fopen();
    fp = fopen("log.txt", "a+"); /*open log.txt for writing */
    fprintf(fp, "\n\t// NEW TEST //\t\n");
    return fp;
}

int writeLogs(FILE *fp, STATS stats) {
    fprintf(fp,
        "-----\nMatrix Size: %d\nMatrix Prob: %.6f\nNumber of Threads: %d\nRuntime: %fms\n-----",
        stats.matrix_size, stats.prob, stats.num_threads, stats.runtime);
    return 0;
}

int writeFailure(FILE *fp, STATS stats) {
    fprintf(fp,
        "\n\n!!! FAILURE !!!\n\n-----\nMatrix Size: %d\nMatrix Prob: %.6f\nNumber of Threads: %d\nRuntime: %fms\n-----",
        stats.matrix_size, stats.prob, stats.num_threads, stats.runtime);
    return 0;
}

double getRuntime(double start, double end) {
    return end - start;
}



