#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <vector>
#include <atomic>
#include <chrono>
#include <random>
#include <condition_variable>

using namespace std;

// Task constants
const int NUM_CAMERAS = 6;
const int NUM_ACCELERATORS = 3;
const int PRIORITY_FRAME_INTERVAL = 2;

// Semaphore implementation
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

// Global variables
CountingSemaphore accelerators_sem(NUM_ACCELERATORS);
mutex priority_mutex, normal_mutex;
queue<int> priority_frames;
queue<int> normal_frames;
atomic<int> active_accelerators{NUM_ACCELERATORS};
atomic<bool> system_running{true};

// Frame processing function
void process_frame(int camera_id, int frame_num, bool is_priority) {
    accelerators_sem.acquire();
    
    int process_time = is_priority ? 1 : 2;
    cout << "Camera " << camera_id << " [" << (is_priority ? "PRIORITY" : "normal") 
         << "] frame " << frame_num << " processing (" << process_time << "s)\n";
    
    this_thread::sleep_for(chrono::seconds(process_time));
    
    accelerators_sem.release();
    
    cout << "Camera " << camera_id << " frame " << frame_num << " processed\n";
}

// System monitoring
void system_monitor() {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> check_interval(5, 10);
    
    while (system_running) {
        this_thread::sleep_for(chrono::seconds(check_interval(gen)));
        
        // 10% chance of accelerator failure
        if (uniform_int_distribution<>(1, 10)(gen) == 1 && active_accelerators > 1) {
            accelerators_sem.acquire();
            active_accelerators--;
            cout << "!!! Accelerator failure! Remaining: " << active_accelerators << endl;
        }
        
        // 20% chance of recovery
        if (active_accelerators < NUM_ACCELERATORS && uniform_int_distribution<>(1, 5)(gen) == 1) {
            accelerators_sem.release();
            active_accelerators++;
            cout << "!!! Accelerator recovered! Total: " << active_accelerators << endl;
        }
    }
}

// Camera function
void camera(int id) {
    int frame_num = 0;
    while (frame_num < 10) {
        frame_num++;
        bool is_priority = (frame_num % PRIORITY_FRAME_INTERVAL) == 0;
        
        if (is_priority) {
            lock_guard<mutex> lock(priority_mutex);
            priority_frames.push(frame_num);
        } else {
            lock_guard<mutex> lock(normal_mutex);
            normal_frames.push(frame_num);
        }
        
        process_frame(id, frame_num, is_priority);
        
        if (is_priority) {
            lock_guard<mutex> lock(priority_mutex);
            priority_frames.pop();
        } else {
            lock_guard<mutex> lock(normal_mutex);
            normal_frames.pop();
        }
        
        this_thread::sleep_for(chrono::milliseconds(300));
    }
}

// Task dispatcher
void task_dispatcher() {
    while (system_running) {
        // Check priority queue
        {
            lock_guard<mutex> lock(priority_mutex);
            if (!priority_frames.empty()) {
                continue;
            }
        }
        
        // Check normal queue
        {
            lock_guard<mutex> lock(normal_mutex);
            if (!normal_frames.empty()) {
                continue;
            }
        }
        
        this_thread::sleep_for(chrono::milliseconds(100));
    }
}

int main() {
    cout << "Video processing system started\n";
    cout << "Cameras: " << NUM_CAMERAS << endl;
    cout << "Accelerators: " << NUM_ACCELERATORS << endl;
    
    thread monitor(system_monitor);
    thread dispatcher(task_dispatcher);
    
    vector<thread> cameras;
    for (int i = 0; i < NUM_CAMERAS; ++i) {
        cameras.emplace_back(camera, i);
    }
    
    for (auto& cam : cameras) {
        cam.join();
    }
    
    system_running = false;
    monitor.join();
    dispatcher.join();
    
    cout << "All frames processed. System shutdown.\n";
    return 0;
}