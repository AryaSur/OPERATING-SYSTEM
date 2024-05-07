#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <chrono>
#include <ctime>
#include <vector>
#include <random>
#include <string>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

const int μCS = 10; 
const int μRem = 5; 

sem_t reader_mutex, rw_mutex, turnstile;
int readers = 0;

string getSysTime() {
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    return string(ctime(&now_c));
}

int exponential(int mean) {
    default_random_engine generator;
    exponential_distribution<double> distribution(1.0 / mean);
    return distribution(generator);
}

// Function to get suffix based on the number
string getSuffix(int num) {
    if (num == 1)
        return "st";
    else if (num == 2)
        return "nd";
    else if (num == 3)
        return "rd";
    else
        return "th";
}

// Writer thread function
void writer(int id, int kw, ofstream& outFile, vector<double>& times) {
    for (int i = 0; i < kw; i++) {
        string reqTime = getSysTime();
        outFile << i + 1 << getSuffix(i + 1) << " CS Request by Writer Thread " << id << " at " << reqTime;

        sem_wait(&turnstile);
        sem_wait(&rw_mutex);

        string enterTime = getSysTime();
        outFile << i + 1 << getSuffix(i + 1) << " CS Entry by Writer Thread " << id << " at " << enterTime;

        sem_post(&turnstile);

        auto start = chrono::steady_clock::now();
        sleep(exponential(μCS));

        auto end = chrono::steady_clock::now();
        double elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        times.push_back(elapsed);

        sem_post(&rw_mutex);
        sleep(exponential(μRem));
    }
}

// Reader thread function
void reader(int id, int kr, ofstream& outFile) {
    for (int i = 0; i < kr; i++) {
        string reqTime = getSysTime();
        outFile << i + 1 << getSuffix(i + 1) << " CS Request by Reader Thread " << id << " at " << reqTime;

        sem_wait(&turnstile);
        sem_wait(&reader_mutex);
        readers++;
        if (readers == 1)
            sem_wait(&rw_mutex);
        sem_post(&turnstile);
        sem_post(&reader_mutex);

        string enterTime = getSysTime();
        outFile << i + 1 << getSuffix(i + 1) << " CS Entry by Reader Thread " << id << " at " << enterTime;

        sem_wait(&reader_mutex);
        readers--;
        if (readers == 0)
            sem_post(&rw_mutex);
        sem_post(&reader_mutex);
        sleep(exponential(μCS));
        sleep(exponential(μRem));
    }
}

int main() {
    ifstream inFile("inp.txt");
    int nw, nr, kw, kr;
    inFile >> nw >> nr >> kw >> kr;
    inFile.close();
    sem_init(&reader_mutex, 0, 1);
    sem_init(&rw_mutex, 0, 1);
    sem_init(&turnstile, 0, 1);
    ofstream outFileRW("RW-log.txt");
    vector<double> writerTimes;

    // Create writer threads
    vector<thread> writerThreads;
    for (int i = 0; i < nw; i++) {
        writerThreads.emplace_back(writer, i + 1, kw, ref(outFileRW), ref(writerTimes));
    }

    // Create reader threads
    vector<thread> readerThreads;
    for (int i = 0; i < nr; i++) {
        readerThreads.emplace_back(reader, i + 1, kr, ref(outFileRW));
    }

    // Join writer threads
    for (auto& th : writerThreads) {
        th.join();
    }

    // Join reader threads
    for (auto& th : readerThreads) {
        th.join();
    }
    outFileRW.close();
    sem_destroy(&reader_mutex);
    sem_destroy(&rw_mutex);
    sem_destroy(&turnstile);

    // Calculate average writer time
    double totalWriterTime = 0;
    for (auto time : writerTimes) {
        totalWriterTime += time;
    }
    double avgWriterTime = totalWriterTime / writerTimes.size();

    // Write average writer time to a separate file
    ofstream outFileAvgTime("AverageTimeRW.txt");
    outFileAvgTime << "Average writer time: " << avgWriterTime << " ms" << endl;
    outFileAvgTime.close();

    return 0;
}
