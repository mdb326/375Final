#include <iostream>
#include <vector>
#include <optional>
#include <functional>
#include <random>
#include <shared_mutex>
#include <thread>
#include <mutex>

template <typename T>
class ConcurrentList {
public:
    ConcurrentList();
    ConcurrentList(int _size);
    bool set(int index, T value);
    T get(int index);
    int size();
    bool contains(T value);
    void display();
    void add(T value);

private:
    int maxSize;
    std::vector<T> data;
    std::vector<std::shared_ptr<std::shared_mutex>> locks;
    std::mutex add_mutex;


    void resize(int newSize);
};

template <typename T>
ConcurrentList<T>::ConcurrentList() {
    maxSize = 16;
    data.resize(maxSize);
    locks.resize(maxSize);
    for(int i = 0; i < maxSize; i++){
        locks[i] = std::make_shared<std::shared_mutex>();
    }
}

template <typename T>
ConcurrentList<T>::ConcurrentList(int _size) {
    maxSize = _size;
    data.resize(maxSize);
    locks.resize(maxSize);
    for(int i = 0; i < maxSize; i++){
        locks[i] = std::make_shared<std::shared_mutex>();
    }
}

template <typename T>
bool ConcurrentList<T>::set(int index, T value) {
    if (index >= 0 && index < maxSize) {
        std::unique_lock<std::shared_mutex> lock(*(locks[index]));
        data[index] = value;
        return true;
    }
    return false;
}

template <typename T>
T ConcurrentList<T>::get(int index) {
    if (index >= 0 && index < maxSize) {
        std::shared_lock<std::shared_mutex> lock(*(locks[index]));
        return data[index];
    }
    throw std::out_of_range("Index out of range");
}

template <typename T>
int ConcurrentList<T>::size() {
    return maxSize;
}

template <typename T>
bool ConcurrentList<T>::contains(T value) {
    for (int i = 0; i < data.size(); i++) {
        std::shared_lock<std::shared_mutex> lock(*(locks[i]));
        if (data[i] == value) {
            return true;
        }
    }
    return false;
}

template <typename T>
void ConcurrentList<T>::display() {
    for (const auto& elem : data) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}

template <typename T>
void ConcurrentList<T>::resize(int newSize) {
    if (newSize < 0) {
        throw std::invalid_argument("New size cannot be negative");
    }

    // Step 1: Lock all existing locks exclusively to prevent data races
    std::vector<std::unique_lock<std::shared_mutex>> heldLocks;
    for (int i = 0; i < maxSize; ++i) {
        heldLocks.emplace_back(*locks[i]); // exclusive lock via RAII
    }

    // Step 2: Resize the data vector
    data.resize(newSize);

    // Step 3: Resize and initialize the locks vector if needed
    if (newSize > maxSize) {
        locks.resize(newSize);
        for (int i = maxSize; i < newSize; ++i) {
            locks[i] = std::make_shared<std::shared_mutex>();
        }
    } else {
        locks.resize(newSize); // safely shrink locks
    }

    // Step 4: Update maxSize
    maxSize = newSize;

    // Step 5: Locks are automatically released when `heldLocks` goes out of scope
}


template <typename T>
void ConcurrentList<T>::add(T value) {
    std::lock_guard<std::mutex> lock(add_mutex);
    if (data.size() < maxSize) {
        data.push_back(value);
    } else {
        resize(maxSize * 2);
        maxSize *= 2;
        data.push_back(value);
    }
}