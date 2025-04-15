#include <iostream> 
#include <boost/thread.hpp> 
void thread_function() { 
    std::cout << "Hello from thread!" << std::endl; 
} 
int main() { 
    std::cout << "Hello from main!" << std::endl; 
    boost::thread t(thread_function); 
    t.join(); 
    return 0; 
} 