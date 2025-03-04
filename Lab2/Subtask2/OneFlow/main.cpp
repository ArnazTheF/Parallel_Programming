#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <fstream> 
#include <cmath>

#include <omp.h>


const double a = -4.0;
const double b = 4.0;
const int nsteps = 40000000;

double func(double x){
    return exp(-x * x);
}

double integrate_omp(double (*func)(double), double a, double b, int n){
    double h = (b-a)/n;
    double sum = 0.0;
    
    #pragma omp parallel num_threads(40)
    {
        double sumloc = 0.0;

        #pragma omp for
        for (int i = 0; i < n; i++){
            sumloc += func(a + h * (i + 0.5));
        }

        #pragma omp atomic
        sum += sumloc;
    }

    sum *= h;
    return sum;
}

int main(){
    for(int i = 0; i < 20; i++){
        double sum = 1.0;
        const auto start = std::chrono::steady_clock::now(); 

        sum = integrate_omp(func, a, b, nsteps);

        const auto end = std::chrono::steady_clock::now(); 
        const std::chrono::duration<double> elapsed_seconds = end - start;

        std::cout << "Time taken for multiplication: " << elapsed_seconds.count() << " seconds." << std::endl;
        //std::cout << sum << std::endl;
        std::ofstream file("second40.csv", std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file for writing." << std::endl;
            return 1;
        }
        file << elapsed_seconds.count() << std::endl;
        file.close();
    }
    return 0;
}