#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define SIZE 9
#define NUM_THREADS 27

int sudoku[SIZE][SIZE];
int results[NUM_THREADS] = {0};

typedef struct {
    int row;
    int col;
    int thread_id;
} parameters;

// Row validation function
void *check_rows(void *arg) {
    parameters *params = (parameters *)arg;
    int r = params->row;
    int check[10] = {0};
    for (int j = 0; j < SIZE; j++) {
        int val = sudoku[r][j];
        if (val < 1 || val > 9 || check[val]) pthread_exit(NULL);
        check[val] = 1;
    }
    results[params->thread_id] = 1;
    pthread_exit(NULL);
}

// Column validation function
void *check_cols(void *arg) {
    parameters *params = (parameters *)arg;
    int c = params->col;
    int check[10] = {0};
    for (int i = 0; i < SIZE; i++) {
        int val = sudoku[i][c];
        if (val < 1 || val > 9 || check[val]) pthread_exit(NULL);
        check[val] = 1;
    }
    results[params->thread_id] = 1;
    pthread_exit(NULL);
}

// 3x3 box validation function
void *check_subgrids(void *arg) {
    parameters *params = (parameters *)arg;
    int r_start = params->row;
    int c_start = params->col;
    int check[10] = {0};
    for (int i = r_start; i < r_start + 3; i++) {
        for (int j = c_start; j < c_start + 3; j++) {
            int val = sudoku[i][j];
            if (val < 1 || val > 9 || check[val]) pthread_exit(NULL);
            check[val] = 1;
        }
    }
    results[params->thread_id] = 1;
    pthread_exit(NULL);
}

int main() {
    printf("--- Parallel Sudoku Validator ---\n");
    printf("Enter the 9x9 grid:\n");
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            scanf("%d", &sudoku[i][j]);

    pthread_t threads[NUM_THREADS];
    int thread_count = 0;

    for (int i = 0; i < SIZE; i++) {
        parameters *data = (parameters *)malloc(sizeof(parameters));
        data->row = i; data->thread_id = thread_count++;
        pthread_create(&threads[data->thread_id], NULL, check_rows, data);
    }

    for (int i = 0; i < SIZE; i++) {
        parameters *data = (parameters *)malloc(sizeof(parameters));
        data->col = i; data->thread_id = thread_count++;
        pthread_create(&threads[data->thread_id], NULL, check_cols, data);
    }

    for (int i = 0; i < 9; i += 3) {
        for (int j = 0; j < 9; j += 3) {
            parameters *data = (parameters *)malloc(sizeof(parameters));
            data->row = i; data->col = j; data->thread_id = thread_count++;
            pthread_create(&threads[data->thread_id], NULL, check_subgrids, data);
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    int is_valid = 1;
    for (int i = 0; i < NUM_THREADS; i++) {
        if (results[i] == 0) is_valid = 0;
    }

    if (is_valid) printf("\nVALID\n");
    else printf("\nINVALID\n");

    return 0;
}