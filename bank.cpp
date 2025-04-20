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
#include <condition_variable>
#include <atomic>

#define ACCOUNTS 1000
#define TOTAL 100000
#define THREADS 16
#define ITERATIONS 2000000 // 2,000,000 total - 100,000 deposit and 1,900,000 balance
#define BALANCETHREADS 13

std::chrono::duration<double> times[THREADS];
std::mutex m;
std::array<std::mutex, ACCOUNTS> mutexes;
std::array<std::shared_mutex, THREADS> threadMutexes;
std::condition_variable balanceCV;

std::atomic<int> balanceCounter = 0;
std::atomic<int> depositCounter = 0;
std::atomic<int> balancesLeft = 0;
std::atomic<bool> finished = false;

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
    int threadAmt = ITERATIONS / iter;
    for(int i = 0; i < iter; i++){
        int choice = generateRandomInt(0,99);
        if(choice < 95){
            depositCounter++;
            deposit(bank, threaded, threadNum);
        }
        else{
            balancesLeft++;
            balanceCV.notify_one();
        }
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    double final_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    double energy_used = (final_power - initial_power) / 1e6; // Convert microjoules to joules
    duration<double> exec_time_i = duration_cast<duration<double>>(t2 - t1);
    times[threadNum] = exec_time_i;
    std::cout << "Thread " << threadNum << " finished in " << exec_time_i.count() << " sec, energy used: " << energy_used << " J\n";
}
void do_work_single(std::map<int, float>& bank, int threadNum, int iter, bool threaded){
    using namespace std::chrono;
    double initial_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    high_resolution_clock::time_point t1 = high_resolution_clock::now();
    int threadAmt = ITERATIONS / iter;
    for(int i = 0; i < iter; i++){
        int choice = generateRandomInt(0,99);
        if(choice < 95){
            deposit(bank, threaded, threadNum);
        }
        else{
            float tot = balance(bank, threaded, THREADS);
            if(tot != TOTAL){
                printf("Balance failed Single: %f\n", tot);
            }
        }
    }
    high_resolution_clock::time_point t2 = high_resolution_clock::now();
    double final_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    double energy_used = (final_power - initial_power) / 1e6; // Convert microjoules to joules
    duration<double> exec_time_i = duration_cast<duration<double>>(t2 - t1);
    times[threadNum] = exec_time_i;
    std::cout << "Thread " << threadNum << " finished in " << exec_time_i.count() << " sec, energy used: " << energy_used << " J\n";
}
void do_work_balance(std::map<int, float>& bank, int threadNum, int iter, bool threaded){
    while(true){
        if(balancesLeft > 0){
            // std::unique_lock<std::mutex> lock(m); // just lock something so there's something to wait for?
            // balanceCV.wait(lock);
        // }
        
        
            balanceCounter++;
            balancesLeft--;
            float tot = balance(bank, threaded, THREADS);
            if(tot != TOTAL){
                printf("Balance failed: %f\n", tot);
            }
        }   
        if(finished){
            return;
        }
    }
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



    for(int i = 0; i < THREADS-BALANCETHREADS; i++){
        threads[i] = std::thread(do_work, std::ref(bank), i, ITERATIONS / (THREADS-BALANCETHREADS), true);
    }
    for(int i = THREADS-BALANCETHREADS; i < THREADS; i++){
        threads[i] = std::thread(do_work_balance, std::ref(bank), i, ITERATIONS / THREADS, true);
    }

    for(unsigned int i = 0; i < THREADS-BALANCETHREADS; i++){ //slow threads
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

    for(int i = 0; i < THREADS-BALANCETHREADS; i++){
        threads[i].join();
    }
    finished = true;
    for(int i = THREADS-BALANCETHREADS; i < THREADS; i++){
        threads[i].join();
    }
    float tot = balance(bank, true, THREADS);
    if(tot != TOTAL){
        printf("Balance failed: %f\n", tot);
    }
    else {
        std::cout << "SUCCESS" << std::endl;
    }

    std::cout << "---------" << std::endl;
    double maxTime = 0.0;
    for(int i = 0; i < THREADS-BALANCETHREADS; i++){
        if(times[i].count() > maxTime){
            maxTime = times[i].count();
        }
    }

    printf("Total %d Threaded time: %lf seconds\n", THREADS, maxTime);


    do_work_single(std::ref(bank), 0, ITERATIONS, false);
    printf("Total nonthreaded time: %lf seconds\n", times[0].count());
    auto it = bank.begin();
    while (it != bank.end()) {
        it = bank.erase(it);  // erase returns an iterator to the next element
    }

    double initial_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    std::this_thread::sleep_for(std::chrono::milliseconds(334));
    double final_power = read_power("/sys/class/powercap/intel-rapl:0/energy_uj");
    double energy_used = (final_power - initial_power) / 1e6; // Convert microjoules to joules
    std::cout << "energy used: " << energy_used << " J\n";

    std::cout << "Balances:" << balanceCounter << " Deposits: " << depositCounter << std::endl;
    std::cout << "TOTAL: " << balanceCounter + depositCounter << std::endl;
    std::cout << "LEFT: " << balancesLeft << std::endl;
}