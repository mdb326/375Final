#include <iostream>
#include <pthread.h>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <unistd.h>
#include <time.h>
#include "ArrayList.h"

#define NUM_THREADS 4
#define NUM_ITERATIONS 1000000


int main() {

    ArrayList<int> list(10);
    list.set(0, 5);
    list.set(1, 10);
    list.display();

    return 0;
}
