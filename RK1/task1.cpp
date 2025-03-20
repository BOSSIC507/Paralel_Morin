#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <random>
#include <chrono>

constexpr int NUM_WORKERS = 5;
constexpr int NUM_MACHINES = 3;

std::mutex mtx;
std::condition_variable cv;
std::atomic<int> available_machines(NUM_MACHINES);

void worker(int id) {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Ожидание, пока не освободится станок
    cv.wait(lock, [] { return available_machines.load() > 0; });
    
    // Захват станка
    available_machines.fetch_sub(1);
    std::wcout << L"Рабочий " << id << L" занял станок." << std::endl;
    
    lock.unlock();
    
    // Симуляция работы на станке
    std::this_thread::sleep_for(std::chrono::seconds(1 + rand() % 3));
    
    lock.lock();
    
    // Освобождение станка
    available_machines.fetch_add(1);
    std::wcout << L"Рабочий " << id << L" освободил станок." << std::endl;
    
    cv.notify_one();
}

int main() {
    setlocale(LC_ALL, "");
    std::vector<std::thread> workers;
    
    for (int i = 0; i < NUM_WORKERS; ++i) {
        workers.emplace_back(worker, i + 1);
    }
    
    for (auto& w : workers) {
        w.join();
    }
    
    std::wcout << L"Все рабочие завершили свою работу!" << std::endl;
    return 0;
}