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

// Function to get system time
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
        sem_post(&rw_mutex);

        auto end = chrono::steady_clock::now();
        double elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        times.push_back(elapsed);

        sleep(exponential(μCS));

        sleep(exponential(μRem));
    }
}

// Reader thread function
void reader(int id, int kr, ofstream& outFile) {
    for (int i = 0; i < kr; i++) {
        string reqTime = getSysTime();
        outFile << i + 1 << getSuffix(i + 1) << " CS Request by Reader Thread " << id << " at " << reqTime;

        sem_wait(&turnstile);
        sem_post(&turnstile);

        sem_wait(&reader_mutex);
        readers++;
        if (readers == 1)
            sem_wait(&rw_mutex);
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
    ofstream outFileFairRW("FairRW-log.txt");
    vector<double> writerTimes;
    vector<thread> writerThreadsFairRW;
    for (int i = 0; i < nw; i++) {
        writerThreadsFairRW.emplace_back(writer, i + 1, kw, ref(outFileFairRW), ref(writerTimes));
    }

    // Create reader threads for Fair Reader Writer
    vector<thread> readerThreadsFairRW;
    for (int i = 0; i < nr; i++) {
        readerThreadsFairRW.emplace_back(reader, i + 1, kr, ref(outFileFairRW));
    }

    // Join writer threads for Fair Reader Writer
    for (auto& th : writerThreadsFairRW) {
        th.join();
    }

    // Join reader threads for Fair Reader Writer
    for (auto& th : readerThreadsFairRW) {
        th.join();
    }
    
    outFileFairRW.close();
    sem_destroy(&reader_mutex);
    sem_destroy(&rw_mutex);
    sem_destroy(&turnstile);

    return 0;
}
