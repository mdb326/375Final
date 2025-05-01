#include <iostream>
#include <vector>
#include <optional>
#include <functional>
#include <random>

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

private:
    int maxSize;
    std::vector<T> data;
    // std::vector<std::shared_ptr<std::shared_mutex>> locks;

    void resize(int newSize);
};

template <typename T>
ConcurrentList<T>::ConcurrentList() {
    maxSize = 16;
    data.resize(maxSize);
    // locks.resize(maxSize);
}

template <typename T>
ConcurrentList<T>::ConcurrentList(int _size) {
    maxSize = _size;
    data.resize(maxSize);
    // locks.resize(maxSize);
}

template <typename T>
bool ConcurrentList<T>::set(int index, T value) {
    if (index >= 0 && index < maxSize) {
        data[index] = value;
        return true;
    }
    return false;
}

template <typename T>
T ConcurrentList<T>::get(int index) {
    if (index >= 0 && index < maxSize) {
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
    for (const auto& elem : data) {
        if (elem == value) {
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
    
    maxSize = newSize;
    data.resize(newSize);
}
