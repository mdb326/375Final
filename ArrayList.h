#include <iostream>
#include <vector>
#include <optional>
#include <functional>
#include <random>

template <typename T>
class ArrayList {
public:
    ArrayList();
    ArrayList(int _size);
    bool set(int index, T value);
    T get(int index);
    int size();
    bool contains(T value);
    void display();
    void add (T value);

private:
    int maxSize;
    std::vector<T> data;

    void resize(int newSize);
};

template <typename T>
ArrayList<T>::ArrayList() {
    maxSize = 16;
    data.resize(maxSize);
}

template <typename T>
ArrayList<T>::ArrayList(int _size) {
    maxSize = _size;
    data.resize(maxSize);
}

template <typename T>
bool ArrayList<T>::set(int index, T value) {
    if (index >= 0 && index < maxSize) {
        data[index] = value;
        return true;
    }
    return false;
}

template <typename T>
T ArrayList<T>::get(int index) {
    if (index >= 0 && index < maxSize) {
        return data[index];
    }
    throw std::out_of_range("Index out of range");
}

template <typename T>
int ArrayList<T>::size() {
    return maxSize;
}

template <typename T>
bool ArrayList<T>::contains(T value) {
    for (const auto& elem : data) {
        if (elem == value) {
            return true;
        }
    }
    return false;
}

template <typename T>
void ArrayList<T>::display() {
    for (const auto& elem : data) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}

template <typename T>
void ArrayList<T>::resize(int newSize) {
    if (newSize < 0) {
        throw std::invalid_argument("New size cannot be negative");
    }
    
    maxSize = newSize;
    data.resize(newSize);
}

template <typename T>
void ArrayList<T>::add(T value) {
    if (data.size() < maxSize) {
        data.push_back(value);
    } else {
        resize(maxSize * 2);
        maxSize *= 2;
        data.push_back(value);
    }
}