#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <random>

constexpr int num_blanks = 10;  // Количество заготовок
constexpr int num_machines = 3; // Количество машин

std::mutex machine_mtx;
std::condition_variable machine_cv;
int available_machines = num_machines; // Количество доступных машин

void process_blank(int blank_id) {
    // Ожидание доступной машины

    std::unique_lock<std::mutex> lock(machine_mtx);
    machine_cv.wait(lock, [] { return available_machines > 0; });
    --available_machines; // Занимаем машину
    
    std::wcout << L"Заготовка " << blank_id << L" загружена в машину." << std::endl;
    lock.unlock();
    // Симуляция обработки (от 1 до 2 секунд)
    std::this_thread::sleep_for(std::chrono::seconds(1 + rand() % 2));

    lock.lock();
    ++available_machines; // Освобождаем машину
    //lock.unlock();
    machine_cv.notify_one(); // Уведомляем другие потоки

    std::wcout << L"Заготовка " << blank_id << L" обработана и возвращена на склад." << std::endl;
    lock.unlock();
}

int main() {
    std::vector<std::thread> workers;
    setlocale(LC_ALL, "");
    // Запускаем потоки для обработки заготовок
    for (int i = 1; i <= num_blanks; ++i) {
        workers.emplace_back(process_blank, i);
    }

    // Ждем завершения всех потоков
    for (auto& w : workers) {
        w.join();
    }

    std::wcout << L"Все заготовки обработаны!" << std::endl;
    return 0;
}