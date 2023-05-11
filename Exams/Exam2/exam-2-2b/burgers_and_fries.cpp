#include <iostream>
#include <thread>
#include <semaphore.h>
#include <mutex>
#include <unistd.h>
using namespace std;
#define MAX_THREADS 100

#define BURGER 0
#define FRIES 1
const char* type_names[] = {"BURGER", "FRIES"};
#define pii pair<int, int>

int k;
mutex m;
sem_t sem_bf;

int waiting_burgers = 0;
int waiting_fries = 0;
int last_ordered;
// Do not change
void process_order() {
    sleep(2);
}

void place_order(int type) {
    std::unique_lock<mutex> ul(m);

    if (k <= 0) {
        if (type == BURGER) {
            waiting_burgers++;
        }
        else if (type == FRIES) {
            waiting_fries++;
        }
        cout << "Waiting: " << type_names[type] << endl;
    }
    else {
        cout << "Order: " << type_names[type] << endl;
        sem_wait(&sem_bf);
        k--;
        last_ordered = type;
    }

    /**
     *  Add logic for synchronization before order processing
     *  Check if already k orders in process; 
     *     if true -> wait (print waiting)
     *     otherwise place this order (print order)
     *  Use type_names[type] to print the order type
     */
    ul.unlock();

    process_order();        // Do not remove, simulates preparation

    ul.lock();
    /**
     *  Add logic for synchronization after order processed
     *  Allow next order of the same type to proceed if there is any waiting; if not, allow the other type to proceed.
     */
    if (waiting_burgers || waiting_fries) {
        if (last_ordered == 0) {
            waiting_burgers--;
        }
        else if (last_ordered == 1) {
            waiting_fries--;
        }
    }
    k++;
    sem_post(&sem_bf);

    ul.unlock();
}

int main() {
    // Initialize necessary variables, semaphores etc.
    sem_init(&sem_bf, 0, k);

    // Read data: done for you, do not change
    pii incoming[MAX_THREADS];
    int _type, _arrival;
    int t;
    cin >> k;
    cin >> t;
    for (int i = 0; i < t; ++i) {
        cin >> _type >> _arrival;
        incoming[i].first = _type;
        incoming[i].second = _arrival;
    }

    //sem_init(&f, 0, k);

    // Create threads: done for you, do not change
    thread* threads[MAX_THREADS];
    for (int i = 0; i < t; ++i) {
        _type = incoming[i].first;
        threads[i] = new thread(place_order, _type);
        if (i < t - 1) {
            int _sleep = incoming[i + 1].second - incoming[i].second;
            sleep(_sleep);
        }
    }

    // Join threads: done for you, do not change
    for (int i = 0; i < t; ++i) {
        threads[i]->join();
        delete threads[i];
    }
    return 0;
}