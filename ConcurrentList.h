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
    int stripeFactor;
    std::vector<T> data;
    std::vector<std::shared_ptr<std::shared_mutex>> locks;
    std::mutex add_mutex;


    void resize(int newSize);
};

template <typename T>
ConcurrentList<T>::ConcurrentList() {
    maxSize = 16;
    stripeFactor = 64;
    data.resize(maxSize);
    locks.resize(maxSize / stripeFactor);
    for(int i = 0; i < maxSize / stripeFactor; i++){
        locks[i] = std::make_shared<std::shared_mutex>();
    }
}

template <typename T>
ConcurrentList<T>::ConcurrentList(int _size) {
    maxSize = _size;
    stripeFactor = 64;
    data.resize(maxSize);
    locks.resize(maxSize / stripeFactor);
    for(int i = 0; i < maxSize / stripeFactor; i++){
        locks[i] = std::make_shared<std::shared_mutex>();
    }
}

template <typename T>
bool ConcurrentList<T>::set(int index, T value) {
    if (index >= 0 && index < maxSize) {
        std::unique_lock<std::shared_mutex> lock(*(locks[index/stripeFactor]));
        data[index] = value;
        return true;
    }
    return false;
}

template <typename T>
T ConcurrentList<T>::get(int index) {
    if (index >= 0 && index < maxSize) {
        std::shared_lock<std::shared_mutex> lock(*(locks[index/stripeFactor]));
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
    for (int stripe = 0; stripe < locks.size(); stripe++) {
        std::shared_lock<std::shared_mutex> lock(*locks[stripe]);
        int end = (stripe * stripeFactor + stripeFactor < data.size()) ? (stripe * stripeFactor + stripeFactor) : data.size();
        for (int i = stripe * stripeFactor; i < end; ++i) {
            if (data[i] == value) {
                return true;
            }
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

    std::vector<std::unique_lock<std::shared_mutex>> heldLocks;
    for (int i = 0; i < maxSize/stripeFactor; ++i) {
        heldLocks.emplace_back(*locks[i]);
    }

    data.resize(newSize);

    if (newSize > maxSize) {
        locks.resize(newSize/stripeFactor);
        for (int i = maxSize/stripeFactor; i < newSize; ++i) {
            locks[i] = std::make_shared<std::shared_mutex>();
        }
    } else {
        locks.resize(newSize/stripeFactor); // safely shrink locks
    }

    maxSize = newSize;
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