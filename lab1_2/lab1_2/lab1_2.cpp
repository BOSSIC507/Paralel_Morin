#include <boost/thread.hpp>

//#include <boost/asio/thread_pool.hpp>
//#include <boost/asio/post.hpp>
//#include <boost/asio.hpp>

#include <iostream>
#include <string>
#include <mutex>
#include <vector>
#include <chrono>

#include <boost/algorithm/string.hpp>
#include <boost/locale.hpp>

// Рекурсивное вычисление числа Фибоначчи

long long fibonacci(int n) {
	
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// Функция-обёртка для каждого потока
void calculate_fibonacci(int n, int thread_id) {
	
    std::cout << "Flow " << thread_id << " counts Fibonacci(" << n << ")...\n";
    long long result = fibonacci(n);
    std::cout << "Flow " << thread_id << " completed. Result: " << result << "\n";
}

int main() {
	
    const int M = 40; // Вычисляем 40-е число Фибоначчи
    const int N = 4;  // Количество потоков

    // Многопоточное выполнение
    auto start = std::chrono::high_resolution_clock::now();
    boost::thread_group threads;

    for (int i = 0; i < N; ++i) {
        threads.create_thread(boost::bind(calculate_fibonacci, M, i));
    }

    threads.join_all();
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Multithreaded execution took: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " mc\n";

    // Последовательное выполнение
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        calculate_fibonacci(M, i);
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "Sequential execution took: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " mc\n";

    return 0;
}
