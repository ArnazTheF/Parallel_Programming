#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <fstream> 

#include <omp.h>

#ifdef USE_BIG
    #define SIZE 40000
#else
    #define SIZE 20000
#endif


std::vector<double> multiplication(const std::vector<double>& vector, const std::vector<double>& matrix) {
    std::vector<double> result(SIZE, 0);
    for (int ij = 0; ij < SIZE*SIZE; ij++) {
        int i = ij/SIZE;
        int j = ij%SIZE;
        result[i] += vector[i] * matrix[i * SIZE + j];
    }
    return result;
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::vector<double> vector(SIZE);
    std::vector<double> matrix(SIZE * SIZE);

    for (int i = 0; i < SIZE; i++) {
        vector[i] = std::rand() % 100;
    }

    for (int i = 0; i < SIZE; ++i) {
        for (int j = 0; j < SIZE; ++j) {
            matrix[i * SIZE + j] = std::rand() % 100;
        }
    }

    for(int i = 0; i < 20; i++){
        const auto start = std::chrono::steady_clock::now(); 

        std::vector<double> result = multiplication(vector, matrix);

        const auto end = std::chrono::steady_clock::now(); 
        const std::chrono::duration<double> elapsed_seconds = end - start;

        std::cout << "Time taken for multiplication: " << elapsed_seconds.count() << " seconds." << std::endl;
        std::ofstream file("40000first1.csv", std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file for writing." << std::endl;
            return 1;
        }
        file << elapsed_seconds.count() << std::endl;
        file.close();
    }
    
    return 0;
}