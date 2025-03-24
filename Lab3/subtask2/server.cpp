#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <unordered_map>
#include <mutex>
#include <condition_variable>
#include <future>
#include <cmath>
#include <random>

template<typename T>
class Server {
public:
    Server() : running(false) {}

    void start() {
        running = true;
        server_thread = std::thread(&Server::process_tasks, this);
    }

    void stop() {
        running = false;
        cv.notify_all();
        if (server_thread.joinable()) {
            server_thread.join();
        }
    }

    size_t add_task(std::function<T()> task) {
        std::unique_lock<std::mutex> lock(task_mutex);
        size_t id = next_task_id++;
        task_queue.push({id, task});
        cv.notify_one();
        return id;
    }

    T request_result(size_t id) {
        std::future<T> future;
        {
            std::unique_lock<std::mutex> lock(result_mutex);
            future = results[id].get_future();
        }
        return future.get();
    }

private:
    void process_tasks() {
        while (running) {
            std::unique_lock<std::mutex> lock(task_mutex);
            cv.wait(lock, [this]() { return !task_queue.empty() || !running; });

            if (!running && task_queue.empty()) {
                break;
            }

            auto task = task_queue.front();
            task_queue.pop();
            lock.unlock();

            T result = task.second();

            std::unique_lock<std::mutex> res_lock(result_mutex);
            results[task.first].set_value(result);
        }
    }

    std::thread server_thread;
    bool running;

    std::queue<std::pair<size_t, std::function<T()>>> task_queue;
    std::unordered_map<size_t, std::promise<T>> results;
    size_t next_task_id = 0;

    std::mutex task_mutex;
    std::mutex result_mutex;
    std::condition_variable cv;
};


void client_sin(Server<double>& server, int N, const std::string& filename) {
    std::ofstream file(filename);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(-3.14, 3.14);

    for (int i = 0; i < N; ++i) {
        double arg = dis(gen);
        size_t id = server.add_task([arg]() { return std::sin(arg); });
        double result = server.request_result(id);
        file << "sin(" << arg << ") = " << result << std::endl;
    }
}

void client_sqrt(Server<double>& server, int N, const std::string& filename) {
    std::ofstream file(filename);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1000);

    for (int i = 0; i < N; ++i) {
        double arg = dis(gen);
        size_t id = server.add_task([arg]() { return std::sqrt(arg); });
        double result = server.request_result(id);
        file << "sqrt(" << arg << ") = " << result << std::endl;
    }
}

void client_pow(Server<double>& server, int N, const std::string& filename) {
    std::ofstream file(filename);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(1, 10);

    for (int i = 0; i < N; ++i) {
        double base = dis(gen);
        double exp = dis(gen);
        size_t id = server.add_task([base, exp]() { return std::pow(base, exp); });
        double result = server.request_result(id);
        file << base << "^" << exp << " = " << result << std::endl;
    }
}

int main() {
    Server<double> server;
    server.start();

    const int N = 10000;
    std::thread client1([&server, N]() { client_sin(server, N, "sin_results.txt"); });
    std::thread client2([&server, N]() { client_sqrt(server, N, "sqrt_results.txt"); });
    std::thread client3([&server, N]() { client_pow(server, N, "pow_results.txt"); });

    client1.join();
    client2.join();
    client3.join();

    server.stop();

    return 0;
}