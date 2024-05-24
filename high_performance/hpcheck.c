#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <bpkg.h>

int main() {
    const char *filename = "tests/data/file1.bpkg";
    const char *log_filename = "high_performance/log.txt";
    clock_t start, end;
    double cpu_time_used;

    start = clock();
    struct bpkg_obj* obj = bpkg_load(filename);
    end = clock();

    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;

    if (obj == NULL) {
        fprintf(stderr, "Failed to load package from %s\n", filename);
        return 1;
    }

    printf("bpkg_load execution time: %f seconds\n", cpu_time_used);

    // FILE *log_file = fopen(log_filename, "a");
    // if (log_file == NULL) {
    //     fprintf(stderr, "Failed to open log file %s\n", log_filename);
    //     return 1;
    // }

    // fprintf(log_file, "bpkg_load execution time: %f seconds\n", cpu_time_used);
    // fclose(log_file);

    bpkg_obj_destroy(obj);

    return 0;
}
