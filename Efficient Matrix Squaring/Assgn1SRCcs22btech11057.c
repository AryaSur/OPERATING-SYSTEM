#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_N 1000

// Function to check if a number is a tetrahedral number
int IsTetrahedral(int num) {
    int p = 1;
    while ((p * (p + 1) * (p + 2)) / 6 < num) {
        p++;
    }
    return (p * (p + 1) * (p + 2)) / 6 == num;
}

// Function for child process
void ChildProcess(int* SharedNumbers, int Start, int End, int* SharedResults, int processNum) {
    char fileName[20];
    sprintf(fileName, "OutFile%d.txt", processNum);
    FILE* logFile = fopen(fileName, "w");

    // Perform number checking in shared memory
    for (int i = Start; i <= End; i++) {
        fprintf(logFile, "%d: %s\n", SharedNumbers[i], IsTetrahedral(SharedNumbers[i]) ? "a tetrahedral number" : "Not a tetrahedral number");
        if (IsTetrahedral(SharedNumbers[i])) {
            SharedResults[i] = SharedNumbers[i];
        } else {
            SharedResults[i] = 0;
        }
    }

    fclose(logFile);
}

int main() {
    clock_t start_time, end_time;

    start_time = clock(); // Record the start time

    int N, K;

    // Read N and K from input file
    FILE* inputFile = fopen("input.txt", "r");
    if (inputFile == NULL) {
        perror("Error opening input.txt");
        exit(EXIT_FAILURE);
    }

    if (fscanf(inputFile, "%d %d", &N, &K) != 2) {
        fprintf(stderr, "Error reading from input.txt\n");
        fclose(inputFile);
        exit(EXIT_FAILURE);
    }

    fclose(inputFile);

    // Create shared memory to store the results
    int shm_fd = shm_open("/myshm", O_CREAT | O_RDWR, S_IRWXU | S_IRWXG);
    ftruncate(shm_fd, sizeof(int) * N * 2);
    int* SharedMemory = mmap(NULL, sizeof(int) * N * 2, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    int* SharedNumbers = SharedMemory;
    int* SharedResults = SharedMemory + N;

    // Initialize the numbers in shared memory
    for (int i = 0; i < N; i++) {
        SharedNumbers[i] = i + 1;
    }

    // Create child processes
    for (int i = 0; i < K; i++) {
        int pid = fork();
        if (pid == 0) {
            // Child process
            int Start = (N / K) * i;
            int End = (i == K - 1) ? N - 1 : (N / K) * (i + 1) - 1;

            ChildProcess(SharedNumbers, Start, End, SharedResults, i + 1);

            exit(0);
        } else if (pid < 0) {
            perror("Fork failed");
            exit(EXIT_FAILURE);
        }
    }

    // Main process waits for all child processes to finish
    for (int i = 0; i < K; i++) {
        wait(NULL);
    }

    end_time = clock(); // Record the end time

    // Calculate elapsed time in seconds
    double elapsed_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

    // Print the elapsed time
    printf("Elapsed Time: %.4f seconds\n", elapsed_time);

    // Consolidate results in OutMain file
    FILE* outFileMain = fopen("OutMain.txt", "w");
    for (int i = 0; i < N; i++) {
        if (SharedResults[i] != 0) {
            int processNum = i / (N / K) + 1;
            fprintf(outFileMain, "P%d:- %d\n", processNum, SharedResults[i]);
        }
    }
    fclose(outFileMain);

    // Clean up shared memory
    munmap(SharedMemory, sizeof(int) * N * 2);
    shm_unlink("/myshm");

    return 0;
}
