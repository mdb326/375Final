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

private:
    int maxSize;
    std::vector<T> data;  // This should hold the actual elements
};

template <typename T>
ArrayList<T>::ArrayList() {
    maxSize = 16;
    data.resize(maxSize);  // Resize the vector to the max size
}

template <typename T>
ArrayList<T>::ArrayList(int _size) {
    maxSize = _size;
    data.resize(maxSize);  // Resize the vector to the specified size
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
