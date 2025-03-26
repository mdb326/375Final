#include <iostream>
#include <pthread.h>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <unistd.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 1000000

pthread_mutex_t mutex_lock;
pthread_spinlock_t spin_lock;

struct ThreadData {
    int id;
    bool use_spinlock;
};

// Function to read power usage from the powercap interface
double read_power(const std::string& power_file) {
    std::ifstream power_stream(power_file);
    double power = 0.0;
    if (power_stream.is_open()) {
        power_stream >> power;
        power_stream.close();
    }
    return power;
}

void* lock_test(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    auto start = std::chrono::high_resolution_clock::now();

    // Record the power usage before starting
    double initial_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");

    for (int i = 0; i < NUM_ITERATIONS; i++) {
        if (data->use_spinlock) {
            pthread_spin_lock(&spin_lock);
        } else {
            pthread_mutex_lock(&mutex_lock);
        }

        // Critical section (simulated work)
        int x = 0;
        for (int j = 0; j < 10; j++) x += j;

        if (data->use_spinlock) {
            pthread_spin_unlock(&spin_lock);
        } else {
            pthread_mutex_unlock(&mutex_lock);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    // Record the power usage after completing the work
    double final_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    double energy_used = (final_power - initial_power) / 1e6; // Convert microjoules to joules

    std::cout << "Thread " << data->id << " finished in " << elapsed.count() << " sec, energy used: " << energy_used << " J\n";

    return nullptr;
}

int main() {
    pthread_mutex_init(&mutex_lock, nullptr);
    pthread_spin_init(&spin_lock, PTHREAD_PROCESS_PRIVATE);

    std::vector<pthread_t> threads(NUM_THREADS);
    std::vector<ThreadData> thread_data(NUM_THREADS);

    // **First test with mutex**
    std::cout << "Running test with Mutex Lock...\n";
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i] = {i, false};
        pthread_create(&threads[i], nullptr, lock_test, &thread_data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    sleep(2); // Small pause before switching to spinlock

    // **Now test with spinlock**
    std::cout << "\nRunning test with Spinlock...\n";
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i] = {i, true};
        pthread_create(&threads[i], nullptr, lock_test, &thread_data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    pthread_mutex_destroy(&mutex_lock);
    pthread_spin_destroy(&spin_lock);

    return 0;
}
