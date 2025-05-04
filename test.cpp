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

#define THREADS 4
#define NUM_ITERATIONS 1000000
#define CONTAINSPER 8
#define ADDSPER 9

std::chrono::duration<double> times[THREADS];
int deltas[THREADS];


int generateRandomVal(int size);
int generateRandomInteger(int min, int max);
void do_work(ConcurrentList<int>& list, int threadNum, int iter, int size);
void do_workSynch(ArrayList<int>& list, int threadNum, int iter, int size);

int main() {

    ArrayList<int> list(10);
    list.set(0, 5);
    list.set(1, 10);
    list.display();

    ConcurrentList<int> list2(10);
    list2.set(0, 5);
    list2.set(1, 10);
    list2.display();

    return 0;
}

void do_work(ConcurrentList<int>& list, int threadNum, int iter, int size){
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= CONTAINSPER) {
            list.contains(generateRandomVal(size));
        } else if (num <= ADDSPER) {
            list.add(generateRandomVal(size));
            deltas[threadNum]++;
        } else {
            if(list.get(generateRandomVal(size))){
                deltas[threadNum]--;
            }
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> exec_time_i = std::chrono::duration_cast<std::chrono::duration<double>>(end - begin);
    times[threadNum] = exec_time_i;
}
void do_workSynch(ArrayList<int>& list, int threadNum, int iter, int size){
    auto begin = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iter; i++) {
        int num = generateRandomInteger(1, 10);
        if (num <= CONTAINSPER) {
            list.contains(generateRandomVal(size));
        } else if (num <= ADDSPER) {
            list.add(generateRandomVal(size));
            deltas[threadNum]++;
        } else {
            if(list.get(generateRandomVal(size))){
                deltas[threadNum]--;
            }
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