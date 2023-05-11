#include <iostream>
// include additional necessary headers
#include <mutex>
#include <thread>
#include <condition_variable>
#include <string>

std::mutex m;
std::condition_variable cv;
bool t_ready = false;

void query(int count) {
    // Should print: the print number (starting from 0), "SYN", and the three dots "..."

    for (int i = 0; i < count; i++) {
        std::unique_lock<std::mutex> ul(m);
        std::cout << "[" << i << "] SYN ... ";
        t_ready = true;
        ul.unlock();
        cv.notify_one();
        ul.lock();
        cv.wait(ul, [] { return t_ready == false; });
    }

}

void response(int count) {
    // Should print "ACK"
    for (int i = 0; i < count; i++) {
        std::unique_lock<std::mutex> ul(m);
        cv.wait(ul, [] {return t_ready; });
        std::cout << "ACK\n";
        t_ready = false;
        ul.unlock();
        cv.notify_one();
    }
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: ./handshake <count>\n");
        exit(1);
    }

    /**
     * Steps to follow:
     * 1. Get the "count" from cmd args
     * 2. Create necessary synchronization primitive(s)
     * 3. Create two threads, one for "SYN" and the other for "ACK"
     * 4. Provide the threads with necessary args
     * 5. Update the "query" and "response" functions to synchronize the output
    */
   int count = std::stoi(argv[1]);
   //std::cout << count << std::endl;
   std::thread syn_thread(query,count);
   std::thread ack_thread(response,count); 

    syn_thread.join();
    ack_thread.join();


    return 0;
}