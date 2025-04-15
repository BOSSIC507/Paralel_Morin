#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <random>
#include <chrono>
#include <atomic>
#include <vector>
#include <condition_variable>

using namespace std;

// Константы задачи
const int NUM_CRANES = 5;         // Количество кранов по умолчанию
const int NUM_TRUCKS = 10;        // Количество грузовиков
const int HIGH_QUEUE_THRESHOLD = 5; // Порог для включения резервного крана
const int LOW_TRUCKS_THRESHOLD = 3; // Порог для аварийной загрузки

// Класс для реализации семафора через mutex и condition_variable
class CountingSemaphore {
private:
    mutex mtx;
    condition_variable cv;
    int count;
public:
    CountingSemaphore(int count = 0) : count(count) {}
    
    void acquire() {
        unique_lock<mutex> lock(mtx);
        cv.wait(lock, [this]() { return count > 0; });
        --count;
    }
    
    void release() {
        lock_guard<mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }
};

// Глобальные переменные
CountingSemaphore cranes_sem(NUM_CRANES); // Наш самодельный семафор
queue<int> truck_queue;
mutex queue_mutex;
atomic<int> active_cranes{NUM_CRANES};
atomic<int> loaded_trucks{0};
atomic<bool> emergency_mode{false};

// Генератор случайных чисел
random_device rd;
mt19937 gen(rd());
uniform_int_distribution<> loading_time(3, 6);

// Функция загрузки грузовика
void load_truck(int truck_id) {
    // Захватываем кран
    cranes_sem.acquire();
    
    // Определяем время загрузки
    int load_time = loading_time(gen);
    if (emergency_mode) {
        load_time = max(1, load_time / 2); // Ускоряем загрузку в аварийном режиме
        cout << "EMERGENCY MODE: Truck " << truck_id << " loading in " << load_time << " seconds\n";
    } else {
        cout << "Truck " << truck_id << " loading in " << load_time << " seconds\n";
    }
    
    // Имитация загрузки
    this_thread::sleep_for(chrono::seconds(load_time));
    
    // Освобождаем кран
    cranes_sem.release();
    
    // Увеличиваем счетчик загруженных грузовиков
    loaded_trucks++;
    cout << "Truck " << truck_id << " loaded. Total loaded: " << loaded_trucks << endl;
    
    // Проверяем условие выхода из аварийного режима
    if (emergency_mode && loaded_trucks >= LOW_TRUCKS_THRESHOLD) {
        emergency_mode = false;
        cout << "Exiting emergency mode\n";
    }
}

// Функция управления кранами
void crane_manager() {
    while (loaded_trucks < NUM_TRUCKS) {
        // Проверяем очередь
        int queue_size;
        {
            lock_guard<mutex> lock(queue_mutex);
            queue_size = truck_queue.size();
        }
        
        // Если очередь большая - добавляем кран
        if (queue_size > HIGH_QUEUE_THRESHOLD && active_cranes < NUM_CRANES + 1) {
            active_cranes++;
            cranes_sem.release();
            cout << "Activated backup crane. Total cranes: " << active_cranes << endl;
        }
        // Если очередь небольшая - убираем лишний кран
        else if (queue_size <= HIGH_QUEUE_THRESHOLD && active_cranes > NUM_CRANES) {
            cranes_sem.acquire();
            active_cranes--;
            cout << "Deactivated backup crane. Total cranes: " << active_cranes << endl;
        }
        
        // Проверяем условие для аварийного режима
        if (loaded_trucks < LOW_TRUCKS_THRESHOLD && !emergency_mode) {
            emergency_mode = true;
            cout << "Entering emergency mode!\n";
        }
        
        this_thread::sleep_for(chrono::seconds(1));
    }
}

// Функция грузовика
void truck(int id) {
    // Добавляем в очередь
    {
        lock_guard<mutex> lock(queue_mutex);
        truck_queue.push(id);
    }
    
    // Загружаемся
    load_truck(id);
    
    // Удаляем из очереди
    {
        lock_guard<mutex> lock(queue_mutex);
        truck_queue.pop();
    }
}

int main() {
    cout << "Port loading system started\n";
    cout << "Initial cranes: " << NUM_CRANES << endl;
    cout << "Trucks to load: " << NUM_TRUCKS << endl;
    
    // Запускаем менеджера кранов
    thread manager(crane_manager);
    
    // Создаем грузовики
    vector<thread> trucks;
    for (int i = 0; i < NUM_TRUCKS; ++i) {
        trucks.emplace_back(truck, i);
        this_thread::sleep_for(chrono::milliseconds(500)); // Небольшая задержка между грузовиками
    }
    
    // Ждем завершения всех грузовиков
    for (auto& t : trucks) {
        t.join();
    }
    
    // Завершаем менеджера
    manager.join();
    
    cout << "All trucks loaded. System shutdown.\n";
    return 0;
}