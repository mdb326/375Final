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
    for(int i = 0; i < maxSize; i++){
        locks[i]->lock_shared();
    }
    for (const auto& elem : data) {
        if (elem == value) {
            for(int i = 0; i < maxSize; i++){
                locks[i]->unlock_shared();
            }
            return true;
        }
    }
    for(int i = 0; i < maxSize; i++){
        locks[i]->unlock_shared();
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
    for(int i = 0; i < maxSize; i++){
        locks[i]->lock_shared();
    }
    maxSize = newSize;
    data.resize(newSize);
    for(int i = 0; i < maxSize; i++){
        locks[i]->unlock_shared();
    }
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