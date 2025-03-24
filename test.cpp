#include <chrono>
#include <iostream>
#include <random>
#include <cstdlib>

#define TESTAMT 20000000

int generateRandomVal(int size);
int generateRandomInteger(int min, int max);

int main(int argc, char* argv[]) {

    auto begin = std::chrono::high_resolution_clock::now();
    
    auto end = std::chrono::high_resolution_clock::now();

    std::cout << "TOTAL EXECUTION TIME = "
              << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "\n";

    return 0;
}

int generateRandomVal(int size) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, size);
    return distrib(gen);
}

int generateRandomInteger(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(min, max);
    return distrib(gen);
}