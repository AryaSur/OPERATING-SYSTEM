#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <atomic>
#include <vector>
#include <unistd.h>
#include <pthread.h>

class TASLock {
public:
    void lock() {}
    void unlock() {}
};

class CASLock {
    std::atomic<int> flag = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (flag.exchange(1, std::memory_order_acquire) == 1) {}
    }

    void unlock() {
        flag.store(0, std::memory_order_release);
    }
};

class BoundedCASLock {
    std::atomic<int> lock_flag = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (lock_flag.exchange(1, std::memory_order_acquire) == 1) {}
    }

    void unlock() {
        lock_flag.store(0, std::memory_order_release);
    }
};

class AtomicIncrement {
public:
    int increment(std::atomic<int>& val) {
        return ++val;
    }
};


void MultiplyChunkTAS(const std::vector<std::vector<int>>& Matrix_A, std::vector<std::vector<int>>& result, int start, int end, std::atomic<int>& counter, int rowInc, TASLock& mutex) {
    int N = Matrix_A.size();
    while (true) {
        mutex.lock();
        int current = counter.fetch_add(rowInc);
        mutex.unlock();
        if (current >= N) break;
        for (int i = current; i < std::min(current + rowInc, N); ++i) {
            for (int j = 0; j < N; ++j) {
                result[i][j] = 0;
                for (int k = 0; k < N; ++k) {
                    result[i][j] += Matrix_A[i][k] * Matrix_A[k][j];
                }
            }
        }
    }
}


void MultiplyChunkCAS(const std::vector<std::vector<int>>& Matrix_A, std::vector<std::vector<int>>& result, int start, int end, std::atomic<int>& counter, int rowInc, CASLock& mutex) {
    int N = Matrix_A.size();
    while (true) {
        mutex.lock();
        int current = counter.fetch_add(rowInc);
        mutex.unlock();
        if (current >= N) break;
        for (int i = current; i < std::min(current + rowInc, N); ++i) {
            for (int j = 0; j < N; ++j) {
                result[i][j] = 0;
                for (int k = 0; k < N; ++k) {
                    result[i][j] += Matrix_A[i][k] * Matrix_A[k][j];
                }
            }
        }
    }
}


void MultiplyChunkBoundedCAS(const std::vector<std::vector<int>>& Matrix_A, std::vector<std::vector<int>>& result, int start, int end, std::atomic<int>& counter, int rowInc, BoundedCASLock& mutex) {
    int N = Matrix_A.size();
    while (true) {
        mutex.lock();
        int current = counter.fetch_add(rowInc);
        mutex.unlock();
        if (current >= N) break;
        for (int i = current; i < std::min(current + rowInc, N); ++i) {
            for (int j = 0; j < N; ++j) {
                result[i][j] = 0;
                for (int k = 0; k < N; ++k) {
                    result[i][j] += Matrix_A[i][k] * Matrix_A[k][j];
                }
            }
        }
    }
}

void MultiplyChunkAtomic(const std::vector<std::vector<int>>& Matrix_A, std::vector<std::vector<int>>& result, int start, int end, std::atomic<int>& counter, int rowInc, AtomicIncrement& increment) {
    int N = Matrix_A.size();
    while (true) {
        int current = increment.increment(counter);
        if (current >= N) break;
        for (int i = current; i < std::min(current + rowInc, N); ++i) {
            for (int j = 0; j < N; ++j) {
                result[i][j] = 0;
                for (int k = 0; k < N; ++k) {
                    result[i][j] += Matrix_A[i][k] * Matrix_A[k][j];
                }
            }
        }
    }
}

int main() {    
    std::string InputFile = "inp.txt";
    std::ifstream infile(InputFile);

    int N, K, rowInc;
    infile >> N >> K >> rowInc;

    std::vector<std::vector<int>> Matrix_A(N, std::vector<int>(N, 0));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            infile >> Matrix_A[i][j];
        }
    }

    infile.close();

    std::vector<std::vector<int>> result(N, std::vector<int>(N, 0));

    int chunkSize = N / K; 

    auto Start_TAS = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> TAS_Threads;
    TASLock tas_mutex;
    std::atomic<int> tas_counter(0);
    for (int i = 0; i < K; ++i) {
        int start = i * chunkSize;
        int end = (i == K - 1) ? N : (i + 1) * chunkSize;
        TAS_Threads.emplace_back([&]() {
            MultiplyChunkTAS(Matrix_A, result, start, end, std::ref(tas_counter), rowInc, tas_mutex);
        });
    }

    for (auto& thread : TAS_Threads) {
        thread.join();
    }
    auto End_TAS = std::chrono::high_resolution_clock::now();
    auto Duration_TAS = std::chrono::duration_cast<std::chrono::microseconds>(End_TAS - Start_TAS);
    auto Start_CAS = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> CAS_Threads;
    CASLock cas_mutex;
    std::atomic<int> cas_counter(0);
    for (int i = 0; i < K; ++i) {
        int start = i * chunkSize;
        int end = (i == K - 1) ? N : (i + 1) * chunkSize;
        CAS_Threads.emplace_back([&]() {
            MultiplyChunkCAS(Matrix_A, result, start, end, std::ref(cas_counter), rowInc, cas_mutex);
        });
    }

    for (auto& thread : CAS_Threads) {
        thread.join();
    }
    auto End_CAS = std::chrono::high_resolution_clock::now();
    auto Duration_CAS = std::chrono::duration_cast<std::chrono::microseconds>(End_CAS - Start_CAS);
    auto Start_BCAS = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> BCAS_Threads;
    BoundedCASLock bcas_mutex;
    std::atomic<int> bcas_counter(0);
    for (int i = 0; i < K; ++i) {
        int start = i * chunkSize;
        int end = (i == K - 1) ? N : (i + 1) * chunkSize;
        BCAS_Threads.emplace_back([&]() {
            MultiplyChunkBoundedCAS(Matrix_A, result, start, end, std::ref(bcas_counter), rowInc, bcas_mutex);
        });
    }

    for (auto& thread : BCAS_Threads) {
        thread.join();
    }
    auto End_BCAS = std::chrono::high_resolution_clock::now();
    auto Duration_BCAS = std::chrono::duration_cast<std::chrono::microseconds>(End_BCAS - Start_BCAS);
    auto Start_Atomic = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> Atomic_Threads;
    AtomicIncrement increment;
    std::atomic<int> atomic_counter(0);
    for (int i = 0; i < K; ++i) {
        int start = i * chunkSize;
        int end = (i == K - 1) ? N : (i + 1) * chunkSize;
        Atomic_Threads.emplace_back([&]() {
            MultiplyChunkAtomic(Matrix_A, result, start, end, std::ref(atomic_counter), rowInc, increment);
        });
    }

    for (auto& thread : Atomic_Threads) {
        thread.join();
    }
    auto End_Atomic = std::chrono::high_resolution_clock::now();
    auto Duration_Atomic = std::chrono::duration_cast<std::chrono::microseconds>(End_Atomic - Start_Atomic);
    std::ofstream Outfile("out.txt");
    Outfile << "Resultant Matrix:\n";
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            Outfile << result[i][j] << " ";
        }
        Outfile << "\n";
    }

    Outfile << "Time Taken (TAS): " << Duration_TAS.count() / 1000000.0 << " seconds\n";
    Outfile << "Time Taken (CAS): " << Duration_CAS.count() / 1000000.0 << " seconds\n";
    Outfile << "Time Taken (Bounded CAS): " << Duration_BCAS.count() / 1000000.0 << " seconds\n";
    Outfile << "Time Taken (Atomic): " << Duration_Atomic.count() / 1000000.0 << " seconds\n";
    Outfile.close();

    return 0;
}