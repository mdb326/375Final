#include <iostream>
#include <pthread.h>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <unistd.h>
#include <time.h>
#include "ArrayList.h"
#include "ConcurrentList.h"

#define THREADS 16
#define NUM_ITERATIONS 100000
#define CONTAINSPER 90
#define ADDSPER 95

std::chrono::duration<double> times[THREADS];


int generateRandomVal(int size);
int generateRandomInteger(int min, int max);
void do_work(ConcurrentList<int>& list, int threadNum, int iter, int size);
void do_workSynch(ArrayList<int>& list, int threadNum, int iter, int size);

int main() {
    int size = 262144;

    ArrayList<int> list1(size);

    ConcurrentList<int> list2(size);

    std::thread threads[THREADS];
    for(int i = 0; i < THREADS; i++){
        threads[i] = std::thread(do_work, std::ref(list2), i, NUM_ITERATIONS/THREADS, size);
    }

    for (auto &th : threads){
        th.join();
    }

    double maxTime = 0.0;
    for(int i = 0; i < THREADS; i++){
        if(times[i].count() > maxTime){
            maxTime = times[i].count();
        }
    }

    printf("Total Parallel %d Threaded time: %lf seconds\n", THREADS, maxTime);


    do_workSynch(std::ref(list1), 0, NUM_ITERATIONS, size);

    printf("Total Sequential time: %lf seconds\n", times[0].count());

    return 0;
}

void do_work(ConcurrentList<int>& list, int threadNum, int iter, int size){
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        int num = generateRandomInteger(1, 100);
        if (num <= CONTAINSPER) {
            list.contains(generateRandomVal(size));
        } else if (num <= ADDSPER) {
            list.set(generateRandomVal(size), generateRandomVal(size));
        } else {
            list.get(generateRandomVal(size)-1);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    times[threadNum] = exec_time_i;
}
void do_workSynch(ArrayList<int>& list, int threadNum, int iter, int size){
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        int num = generateRandomInteger(1, 100);
        if (num <= CONTAINSPER) {
            list.contains(generateRandomVal(size));
        } else if (num <= ADDSPER) {
            list.add(generateRandomVal(size));
        } else {
            list.get(generateRandomVal(size)-1);
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    times[threadNum] = exec_time_i;
}


int generateRandomVal(int size) {
    thread_local static std::random_device rd;
    thread_local static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, size);
    return distrib(gen);
}

int generateRandomInteger(int min, int max) {
    thread_local static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    thread_local static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(min, max); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}