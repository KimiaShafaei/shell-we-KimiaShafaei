#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <time.h>

#define NUM_PROCESSES 4

int main() {
    long total_points;
    printf("Enter total number of points: ");
    if (scanf("%ld", &total_points) != 1) return 1;

    // Creating Shared Memory for Point Counting
    long *circle_points = mmap(NULL, sizeof(long), PROT_READ | PROT_WRITE, 
                              MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *circle_points = 0;

    long points_per_process = total_points / NUM_PROCESSES;

    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid_t pid = fork();
        if (pid == 0) { 
            unsigned int seed = time(NULL) ^ getpid();
            long local_hits = 0;
            for (long j = 0; j < points_per_process; j++) {
                double x = (double)rand_r(&seed) / RAND_MAX;
                double y = (double)rand_r(&seed) / RAND_MAX;
                if (x * x + y * y <= 1.0) local_hits++;
            }
            __sync_fetch_and_add(circle_points, local_hits);
            exit(0);
        }
    }

    for (int i = 0; i < NUM_PROCESSES; i++) wait(NULL);

    double pi = 4.0 * (double)(*circle_points) / total_points;
    printf("Estimated Pi (Process-based): %f\n", pi);

    munmap(circle_points, sizeof(long));
    return 0;
}