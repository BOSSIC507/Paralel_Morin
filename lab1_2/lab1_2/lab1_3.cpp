#include <iostream>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

const int NUM_ITERATIONS = 1000000;

void without_sync(int &counter) {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        ++counter;
    }
}

void with_atomic(std::atomic<int> &counter) {
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        ++counter;
    }
}

void with_mutex(int &counter, std::mutex &mtx) {
    
    for (int i = 0; i < NUM_ITERATIONS; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++counter;
    }
}

int main() {
    {
        // Без синхронизации
        int counter = 0;
        auto start = std::chrono::high_resolution_clock::now();
        std::thread t1(without_sync, std::ref(counter));
        std::thread t2(without_sync, std::ref(counter));
        t1.join();
        t2.join();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Without synchronization: " << counter << " (time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << " mc)\n";
    }

    {
        // С использованием atomic
        std::atomic<int> counter(0);
        auto start = std::chrono::high_resolution_clock::now();
        std::thread t1(with_atomic, std::ref(counter));
        std::thread t2(with_atomic, std::ref(counter));
        t1.join();
        t2.join();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "C std::atomic: " << counter << " (time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << " mc)\n";
    }

    {
        // С использованием mutex
        int counter = 0;
        std::mutex mtx;
        auto start = std::chrono::high_resolution_clock::now();
        std::thread t1(with_mutex, std::ref(counter), std::ref(mtx));
        std::thread t2(with_mutex, std::ref(counter), std::ref(mtx));
        t1.join();
        t2.join();
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "C std::mutex: " << counter << " (time: "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
                  << " mc)\n";
    }

    return 0;
}
