#include <map>
#include <cstdio>
#include <chrono>
#include <thread>
#include <random>
#include <mutex>
#include <shared_mutex>
#include <array>
#include <fstream>
#include <iostream>

#define ACCOUNTS 1000
#define TOTAL 100000
#define THREADS 16
#define ITERATIONS 2000000 // 2,000,000 total - 100,000 deposit and 1,900,000 balance
#define BALANCETHREADS 4

//is there a dfiference between vector and array here?
std::chrono::duration<double> times[THREADS];
//std::mutex m;
std::array<std::mutex, ACCOUNTS> mutexes;
std::array<std::shared_mutex, THREADS> threadMutexes;

//Function to read power usage from the interface
double read_power(const std::string& power_file) {
    std::ifstream power_stream(power_file);
    double power = 0.0;
    if (power_stream.is_open()) {
        power_stream >> power;
        power_stream.close();
    }
    return power;
}

// Generates a random int between min and max (inclusive)
int generateRandomInt(int min, int max) {
    thread_local static std::random_device rd; // creates random device (unique to each thread to prevent race cons) (static to avoid reinitialization)
    thread_local static std::mt19937 gen(rd());  // Seeding the RNG (unique to each thread to prevent race cons) (static to avoid reinitialization)
    std::uniform_int_distribution<> distrib(min, max); // Create uniform int dist between min and max (inclusive)

    return distrib(gen); // Generate random number from the uniform int dist (inclusive)
}

void deposit(std::map<int, float>& bank, bool threaded, int threadNum){
    int acct1 = generateRandomInt(0, ACCOUNTS-1);
    int acct2 = generateRandomInt(0, ACCOUNTS-1);
    while(acct1 == acct2){
        acct2 = generateRandomInt(0, ACCOUNTS-1);
    }
    if(threaded){
        threadMutexes[threadNum].lock();
        //prevent deadlocking
        if(acct1 < acct2){
            mutexes[acct1].lock();
            mutexes[acct2].lock(); 
        }
        else{
            mutexes[acct2].lock(); 
            mutexes[acct1].lock();
        }
    }
    //has to happen before getting the amount because otherwise we could be getting nonexistant amounts
    int amt = generateRandomInt(0, (int)bank[acct1]);
    bank[acct1] -= amt;
    bank[acct2] += amt;
    if(threaded){
        threadMutexes[threadNum].unlock();
        mutexes[acct1].unlock();
        mutexes[acct2].unlock();
    }
}

float balance(std::map<int, float>& bank, bool threaded, int threadAmt){
    float total = 0;    
    if(threaded){
        for(int i = 0; i < threadAmt; i++){
            threadMutexes[i].lock_shared();
        }
    }
    for(auto iter = bank.begin(); iter != bank.end(); iter++){
        total += iter->second;
    }
    if(threaded){
        for(int i = 0; i < threadAmt; i++){
            threadMutexes[i].unlock_shared();
        }
    }
    return total;
}

void do_work(std::map<int, float>& bank, int threadNum, int iter, bool threaded){
    using namespace std::chrono;
    double initial_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int threadAmt = THREADS;
    for(int i = 0; i < iter; i++){
        int choice = generateRandomInt(0,99);
        if(choice < 95){
            deposit(bank, threaded, threadNum);
        }
        else{
            float tot = balance(bank, threaded, threadAmt);
            if(tot != TOTAL){
                printf("Balance failed: %f\n", tot);
            }
        }
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    double final_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    double energy_used = (final_power - initial_power) / 1e6; // Convert microjoules to joules
    duration<double> exec_time_i = duration_cast<duration<double>>(t2 - t1);
    times[threadNum] = exec_time_i;
    std::cout << "Deposit thread " << threadNum << " finished in " << exec_time_i.count() << " sec, energy used: " << energy_used << " J\n";
}

void do_work_deposit(std::map<int, float>& bank, int threadNum, int iter, bool threaded){
    using namespace std::chrono;
    double initial_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int threadAmt = THREADS;
    for(int i = 0; i < iter; i++){
        deposit(bank, threaded, threadNum);

    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    double final_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    double energy_used = (final_power - initial_power) / 1e6; // Convert microjoules to joules
    duration<double> exec_time_i = duration_cast<duration<double>>(t2 - t1);
    times[threadNum] = exec_time_i;
    std::cout << "Deposit thread " << threadNum << " finished in " << exec_time_i.count() << " sec, energy used: " << energy_used << " J\n";
}

void do_work_balance(std::map<int, float>& bank, int threadNum, int iter, bool threaded){
    using namespace std::chrono;
    double initial_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int threadAmt = THREADS;
    for(int i = 0; i < iter; i++){
        float tot = balance(bank, threaded, threadAmt);
        if(tot != TOTAL){
            printf("Balance failed: %f\n", tot);
        }

    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    double final_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    double energy_used = (final_power - initial_power) / 1e6; // Convert microjoules to joules
    duration<double> exec_time_i = duration_cast<duration<double>>(t2 - t1);
    times[threadNum] = exec_time_i;
    std::cout << "Balance Thread " << threadNum << " finished in " << exec_time_i.count() << " sec, energy used: " << energy_used << " J\n";
}

void checkAffinity(int threadNum){
    std::cout << "Thread #" << threadNum << ": on CPU " << sched_getcpu() << "\n";
}

int main(int argc, char **argv) {
    // std::cout << std::thread::hardware_concurrency() << std::endl;
    std::map<int, float> bank; //id, amount
    //fill up accounts
    for(int i = 0; i < ACCOUNTS; i++){
        bank.insert({i, TOTAL / ACCOUNTS});
    }
    //create threads and do their work
    std::thread threads[THREADS];
    unsigned int depositIterations = (ITERATIONS * 95 / 100) / (THREADS-BALANCETHREADS);
    unsigned int balanceIterations = (ITERATIONS * 5 / 100) / BALANCETHREADS;

    for(int i = 0; i < THREADS-BALANCETHREADS; i++){
        threads[i] = std::thread(do_work_deposit, std::ref(bank), i, depositIterations, true);
    }
    for(int i = THREADS-BALANCETHREADS; i < THREADS; i++){
        threads[i] = std::thread(do_work_balance, std::ref(bank), i, balanceIterations, true);
    }

    for(unsigned int i = 0; i < THREADS-BALANCETHREADS; i++){
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(27-i, &cpuset);
        int rc = pthread_setaffinity_np(threads[i].native_handle(),
                                        sizeof(cpu_set_t), &cpuset);
    }

    for(unsigned int i = THREADS-BALANCETHREADS; i < THREADS; i++){ //fast for contains
        cpu_set_t cpuset;
        CPU_ZERO(&cpuset);
        CPU_SET(i, &cpuset);
        int rc = pthread_setaffinity_np(threads[i].native_handle(),
                                        sizeof(cpu_set_t), &cpuset);
    }

    for (auto &th : threads){
        th.join();
    }

    std::cout << "---------" << std::endl;
    double maxTime = 0.0;
    for(int i = 0; i < THREADS; i++){
        if(times[i].count() > maxTime){
            maxTime = times[i].count();
        }
    }

    printf("Total %d Threaded time: %lf seconds\n", THREADS, maxTime);


    do_work(std::ref(bank), 0, ITERATIONS, false);
    printf("Total nonthreaded time: %lf seconds\n", times[0].count());
    auto it = bank.begin();
    while (it != bank.end()) {
        it = bank.erase(it);  // erase returns an iterator to the next element
    }

    double initial_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    std::this_thread::sleep_for(std::chrono::milliseconds(334));
    double final_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    double energy_used = (final_power - initial_power) / 1e6; // Convert microjoules to joules
    std::cout << "energy used in 0.344s: " << energy_used << " J\n";
}