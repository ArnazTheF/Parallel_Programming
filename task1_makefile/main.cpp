#define _USE_MATH_DEFINES

#include <iostream>
#include <cmath>
#include <vector>

template<typename T>
void calculateAndPrintSum() {
    T pi = static_cast<T>(M_PI);
    std::vector<T> divided(1e7, (2 * pi) / 1e7);

    T sum = 0;
    T counter = 0;
    for (T part : divided) {
        sum += std::sin(part * counter);
        counter++;
    }

    std::cout << sum << std::endl;
}

int main() {
    #ifdef FLOAT
        calculateAndPrintSum<float>();
    #else
        calculateAndPrintSum<double>();
    #endif

    return 0;
}