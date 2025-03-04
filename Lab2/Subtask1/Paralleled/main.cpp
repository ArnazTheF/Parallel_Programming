#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <fstream> 
#include <random>

#include <omp.h>

#ifdef USE_BIG
    #define SIZE 40000
#else
    #define SIZE 20000
#endif

#define NUMBER_OF_THREADS 40


std::vector<double> multiplication(const std::vector<double>& vector, const std::vector<double>& matrix) {
    std::vector<double> result(SIZE, 0);
    #pragma omp parallel num_threads(NUMBER_OF_THREADS)
    {
        #pragma omp for
        for (int ij = 0; ij < SIZE*SIZE; ij++) {
            int i = ij/SIZE;
            int j = ij%SIZE;
            result[i] += vector[i] * matrix[i * SIZE + j];
        }
    }
    return result;
}

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    std::vector<double> vector(SIZE);
    std::vector<double> matrix(SIZE * SIZE);

    #pragma omp parallel num_threads(NUMBER_OF_THREADS)
    {
        unsigned int seed = std::rand() + omp_get_thread_num(); // Разные семена для разных потоков
        std::minstd_rand local_gen(seed);


        #pragma omp for
        for (int i = 0; i < SIZE; i++) {
            vector[i] = local_gen() % 100;
        }

        #pragma omp for
        for (int ij = 0; ij < SIZE*SIZE; ++ij) {
            matrix[ij] = local_gen() % 100;
        }
    }

    for(int i = 0; i < 20; i++){
        const auto start = std::chrono::steady_clock::now(); 

        std::vector<double> result = multiplication(vector, matrix);

        const auto end = std::chrono::steady_clock::now(); 
        const std::chrono::duration<double> elapsed_seconds = end - start;

        std::cout << "Time taken for multiplication: " << elapsed_seconds.count() << " seconds." << std::endl;
        std::ofstream file("40000first40.csv", std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file for writing." << std::endl;
            return 1;
        }
        file << elapsed_seconds.count() << std::endl;
        file.close();
    }

    return 0;
}