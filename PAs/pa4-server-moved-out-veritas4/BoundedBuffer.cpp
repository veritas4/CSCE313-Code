#include "BoundedBuffer.h"

using namespace std;


BoundedBuffer::BoundedBuffer (int _cap) : cap(_cap) {
    // modify as needed
}

BoundedBuffer::~BoundedBuffer () {
    // modify as needed
}

void BoundedBuffer::push (void* message, int size) {
    // 1. Convert the incoming byte sequence given by msg and size into a vector<char>
    //cout << "SIZE: " << size << endl;
    char * msg = (char *) message;
    vector<char> msg_vector(msg, msg + size);
    // 2. Wait until there is room in the queue (i.e., queue lengh is less than cap)
    unique_lock<mutex> ul(m);
    push_available.wait(ul, [this] { return q.size() < (size_t) cap; });
    // 3. Then push the vector at the end of the queue
    q.push(msg_vector);
    ul.unlock();
    // 4. Wake up threads that were waiting for push
    pop_available.notify_one();
}

int BoundedBuffer::pop (void* message, int size) {
    // 1. Wait until the queue has at least 1 item
    unique_lock<mutex> ul(m);
    pop_available.wait(ul, [this] { return q.size() >= 1;});
    // 2. Pop the front item of the queue. The popped item is a vector<char>
    vector<char> item = q.front();
    q.pop();
    ul.unlock();
    // 3. Convert the popped vector<char> into a char*, copy that into message; assert that the vector<char>'s length is <= size
    assert(item.size() <= (size_t) size);

    memcpy(message,item.data(),item.size());
    // 4. Wake up threads that were waiting for pop
    push_available.notify_one();
    // 5. Return the vector's length to the caller so that they know how many bytes were popped
    return item.size();
}

size_t BoundedBuffer::size () {
    return q.size();
}
