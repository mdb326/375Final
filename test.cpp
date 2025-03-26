#include <iostream>
#include <pthread.h>
#include <vector>
#include <chrono>
#include <fstream>
#include <string>
#include <unistd.h>
#include <time.h>

#define NUM_THREADS 4
#define NUM_ITERATIONS 1000000

pthread_mutex_t mutex_lock;
pthread_spinlock_t spin_lock;

struct ThreadData {
    int id;
    bool use_spinlock;
    double cpu_time;  // Store per-thread CPU time
};

// Function to read power usage from the powercap interface (total system energy)
double read_energy(const std::string& power_file) {
    std::ifstream power_stream(power_file);
    double energy = 0.0;
    if (power_stream.is_open()) {
        power_stream >> energy;
        power_stream.close();
    }
    return energy;
}

// Function to get per-thread CPU time (in seconds)
double get_thread_cpu_time() {
    struct timespec ts;
    clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void* lock_test(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    auto start_time = std::chrono::high_resolution_clock::now();
    double start_cpu_time = get_thread_cpu_time();

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

    auto end_time = std::chrono::high_resolution_clock::now();
    double end_cpu_time = get_thread_cpu_time();

    std::chrono::duration<double> elapsed = end_time - start_time;
    data->cpu_time = end_cpu_time - start_cpu_time;  // Store per-thread CPU time

    std::cout << "Thread " << data->id << " finished in " << elapsed.count() 
              << " sec, CPU time: " << data->cpu_time << " sec\n";

    return nullptr;
}

int main() {
    pthread_mutex_init(&mutex_lock, nullptr);
    pthread_spin_init(&spin_lock, PTHREAD_PROCESS_PRIVATE);

    std::vector<pthread_t> threads(NUM_THREADS);
    std::vector<ThreadData> thread_data(NUM_THREADS);

    // **Measure total system energy before running threads**
    double initial_energy = read_energy("/sys/class/powercap/intel-rapl:0/energy_uj");

    std::cout << "Running test with Mutex Lock...\n";
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i] = {i, false, 0.0};
        pthread_create(&threads[i], nullptr, lock_test, &thread_data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    // **Measure total system energy after threads finished**
    double final_energy = read_energy("/sys/class/powercap/intel-rapl:0/energy_uj");
    double total_energy_used = (final_energy - initial_energy) / 1e6;  // Convert microjoules to joules

    // **Calculate total CPU time used by all threads**
    double total_cpu_time = 0.0;
    for (const auto& t : thread_data) {
        total_cpu_time += t.cpu_time;
    }

    // **Distribute energy among threads based on CPU time share**
    std::cout << "\nEnergy Distribution Per Thread:\n";
    for (const auto& t : thread_data) {
        double thread_energy = (t.cpu_time / total_cpu_time) * total_energy_used;
        std::cout << "Thread " << t.id << " estimated energy used: " 
                  << thread_energy << " J (CPU time: " << t.cpu_time << " sec)\n";
    }

    sleep(2); // Small pause before switching to spinlock

    // **Repeat for spinlock**
    initial_energy = read_energy("/sys/class/powercap/intel-rapl:0/energy_uj");

    std::cout << "\nRunning test with Spinlock...\n";
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_data[i] = {i, true, 0.0};
        pthread_create(&threads[i], nullptr, lock_test, &thread_data[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], nullptr);
    }

    final_energy = read_energy("/sys/class/powercap/intel-rapl:0/energy_uj");
    total_energy_used = (final_energy - initial_energy) / 1e6;

    total_cpu_time = 0.0;
    for (const auto& t : thread_data) {
        total_cpu_time += t.cpu_time;
    }

    std::cout << "\nEnergy Distribution Per Thread (Spinlock):\n";
    for (const auto& t : thread_data) {
        double thread_energy = (t.cpu_time / total_cpu_time) * total_energy_used;
        std::cout << "Thread " << t.id << " estimated energy used: " 
                  << thread_energy << " J (CPU time: " << t.cpu_time << " sec)\n";
    }

    pthread_mutex_destroy(&mutex_lock);
    pthread_spin_destroy(&spin_lock);

    return 0;
}
