#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <fstream> 
#include <random>
#include <thread>
#include <mutex>

#define SIZE 40000
#define NUM_THREADS 40

/*
std::this_thread::get_id() - возвращает id потока
std::this_thread::sleep_for(std::chrono::milliseconds(1000)) - "усыпляет" поток на указанное время

std::thread th1(указатель на фуцнкцию) - передаем функцию потоку
Для корректной работы программы после выполнения потока делаем либо
th.join()
либо 
th.detach()
при чем .join() вызываем в том месте кода, в котором хотим дождаться выполнения дочернего потока
*/

void initialize_vector(std::vector<double>& vector, int start, int end, std::minstd_rand& gen) {
    for (int i = start; i < end; ++i) {
        vector[i] = gen() % 100;
    }
}

void initialize_matrix(std::vector<double>& matrix, int start, int end, std::minstd_rand& gen) {
    for (int i = start; i < end; ++i) {
        matrix[i] = gen() % 100;
    }
}

void multiply_part(const std::vector<double>& vector, const std::vector<double>& matrix, std::vector<double>& result, int start, int end) {
    for (int i = start; i < end; ++i) {
        result[i] = 0;
        for (int j = 0; j < SIZE; ++j) {
            result[i] += vector[j] * matrix[i * SIZE + j];
        }
    }
}

int main() {
    std::vector<double> vector(SIZE);
    std::vector<double> matrix(SIZE * SIZE);
    std::vector<double> result(SIZE, 0);

    std::minstd_rand gen(std::rand());

    std::vector<std::thread> threads;
    int chunk_size = SIZE / NUM_THREADS;
    for (int i = 0; i < NUM_THREADS; ++i) {
        int start = i * chunk_size;
        int end = (i == NUM_THREADS - 1) ? SIZE : start + chunk_size;
        threads.emplace_back(initialize_vector, std::ref(vector), start, end, std::ref(gen));
    }
    for (auto& t : threads) {
        t.join();
    }
    threads.clear();
 
    chunk_size = (SIZE * SIZE) / NUM_THREADS;
    for (int i = 0; i < NUM_THREADS; ++i) {
        int start = i * chunk_size;
        int end = (i == NUM_THREADS - 1) ? SIZE * SIZE : start + chunk_size;
        threads.emplace_back(initialize_matrix, std::ref(matrix), start, end, std::ref(gen));
    }
    for (auto& t : threads) {
        t.join();
    }
    threads.clear();

    for (int i = 0; i < 20; ++i) {
        const auto start = std::chrono::steady_clock::now();

        chunk_size = SIZE / NUM_THREADS;
        for (int i = 0; i < NUM_THREADS; ++i) {
            int start_idx = i * chunk_size;
            int end_idx = (i == NUM_THREADS - 1) ? SIZE : start_idx + chunk_size;
            threads.emplace_back(multiply_part, std::ref(vector), std::ref(matrix), std::ref(result), start_idx, end_idx);
        }
        for (auto& t : threads) {
            t.join();
        }
        threads.clear();

        const auto end = std::chrono::steady_clock::now();
        const std::chrono::duration<double> elapsed_seconds = end - start;

        std::cout << "Time taken for multiplication: " << elapsed_seconds.count() << " seconds." << std::endl;
        std::ofstream file("40multithreaded40000.csv", std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file for writing." << std::endl;
            return 1;
        }
        file << elapsed_seconds.count() << std::endl;
        file.close();
    }

    return 0;
}