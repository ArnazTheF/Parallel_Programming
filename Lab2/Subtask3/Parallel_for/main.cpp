#include <iostream>
#include <vector>
#include <cstdio>
#include <ctime>
#include <cstdlib>
#include <chrono>
#include <fstream> 
#include <cmath>

#include <omp.h>


const int N = 20000;

void matrixInit(std::vector<double>& A){
    for(int ij = 0; ij < N*N; ij++){
        int i = ij/N;
        int j = ij % N;
        if(i == j){
            A[i*N + j] = 2.0;
        }
        else{
            A[i*N + j] = 1.0;
        }
        
    }
}

void vectorInit(std::vector<double>& B){
    for(int i = 0; i < N; i++){
        B[i] = N+1;
    }
}

double iteration(std::vector<double>& A, std::vector<double>& B, std::vector<double>& xprev, std::vector<double>& AxminB){
        
    double tau = 0.00001;
    double Ax = 0.0;

    double distanceAxminB = 0;
    double distanceB = 0;
    
    #pragma omp parallel for num_threads(20)
    for(int ij = 0; ij < N*N; ij++){
        int i = ij/N;
        int j = ij % N;

        Ax += A[i*N + j] * xprev[i];

        if(j == N-1){

            AxminB[i] = Ax - B[i];
            Ax = 0.0;
            xprev[i] = xprev[i] - tau*AxminB[i];
        }
    }

    #pragma omp parallel for num_threads(20)
    for(int i = 0; i < N; i++){
        distanceAxminB += AxminB[i]*AxminB[i];
        distanceB += B[i]*B[i];
    }
    return sqrt(distanceAxminB)/sqrt(distanceB);


}


int main(){
    std::vector<double> A(N*N);
    std::vector<double> B(N);
    
    std::vector<double> result(N);
    std::vector<double> AxminB(N);

    matrixInit(A);
    vectorInit(B);

    

    for(int i = 0; i < 20; i++){
        std::vector<double> xprev(N, 0);
        double epsilon = 0.00001;
        double error = 1.0;
        
        const auto start = std::chrono::steady_clock::now(); 
        
        
        while(error > epsilon){
            error = iteration(A, B, xprev, AxminB);
        }


        const auto end = std::chrono::steady_clock::now(); 
        const std::chrono::duration<double> elapsed_seconds = end - start;

        std::cout << "Time taken for multiplication: " << elapsed_seconds.count() << " seconds." << std::endl;
        std::cout << "First elem: " << xprev[0] << std::endl;
        std::ofstream file("forthird20.csv", std::ios::app);
        if (!file.is_open()) {
            std::cerr << "Error: Unable to open file for writing." << std::endl;
            return 1;
        }
        file << elapsed_seconds.count() << std::endl;
        file.close();
    }
    
    return 0;
}