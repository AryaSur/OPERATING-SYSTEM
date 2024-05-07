#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <pthread.h> 

void MultiplyChunk(const std::vector<std::vector<int>>& Matrix_A, std::vector<std::vector<int>>& result, int start, int end) {
    int N = Matrix_A.size();
    for (int i = start; i < end; ++i) {
        for (int j = 0; j < N; ++j) {
            result[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                result[i][j] += Matrix_A[i][k] * Matrix_A[k][j];
            }
        }
    }
}


void MultiplyMixed(const std::vector<std::vector<int>>& Matrix_A, std::vector<std::vector<int>>& result, int threadId, int K) {
    int N = Matrix_A.size();
    for (int i = threadId; i < N; i += K) {
        for (int j = 0; j < N; ++j) {
            result[i][j] = 0;
            for (int k = 0; k < N; ++k) {
                result[i][j] += Matrix_A[i][k] * Matrix_A[k][j];
            }
        }
    }
}


void SetThreadAffinity(std::thread& thread, int coreId) {
    pthread_t native_handle = thread.native_handle();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(coreId, &cpuset);
    pthread_setaffinity_np(native_handle, sizeof(cpu_set_t), &cpuset);
}

int main() {
    std::string InputFile = "inp.txt";

    std::ifstream infile(InputFile);

    int N, K, C, BT;
    infile >> N >> K >> C >> BT;

    std::vector<std::vector<int>> Matrix_A(N, std::vector<int>(N, 0));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            infile >> Matrix_A[i][j];
        }
    }

    infile.close();
    std::vector<std::vector<int>> result(N, std::vector<int>(N, 0));
    auto Start_Chunk = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> Chunk_Threads;
    for (int i = 0; i < K; ++i) {
        int start = i * (N / K);
        int end = (i + 1) * (N / K);
        Chunk_Threads.emplace_back(MultiplyChunk, std::ref(Matrix_A), std::ref(result), start, end);
        if (i < BT) {
            SetThreadAffinity(Chunk_Threads.back(), i % C);
        }
    }

    for (auto& thread : Chunk_Threads) {
        thread.join();
    }
    auto End_Chunk = std::chrono::high_resolution_clock::now();
    auto Duration_Chunk = std::chrono::duration_cast<std::chrono::microseconds>(End_Chunk - Start_Chunk);
    auto Start_Mixed = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> Mixed_Threads;
    for (int i = 0; i < K; ++i) {
        Mixed_Threads.emplace_back(MultiplyMixed, std::ref(Matrix_A), std::ref(result), i, K);
        if (i < BT) {
            SetThreadAffinity(Mixed_Threads.back(), i % C);
        }
    }

    for (auto& thread : Mixed_Threads) {
        thread.join();
    }
    auto End_Mixed = std::chrono::high_resolution_clock::now();
    auto Duration_Mixed = std::chrono::duration_cast<std::chrono::microseconds>(End_Mixed - Start_Mixed);

    std::string OutputFile = "out.txt";
    std::ofstream Outfile(OutputFile);
    Outfile << "Resultant Matrix:\n";
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            Outfile << result[i][j] << " ";
        }
        Outfile << "\n";
    }

    Outfile << "Time Taken (Chunk): " << Duration_Chunk.count() / 1000000.0 << " seconds\n";
    Outfile << "Time Taken (Mixed): " << Duration_Mixed.count() / 1000000.0 << " seconds\n";

    Outfile.close();

    return 0;
}
