#ifndef _BOUNDEDBUFFER_H_
#define _BOUNDEDBUFFER_H_

#include <queue>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <assert.h>
#include <string.h>
#include <iostream>

using namespace std;


class BoundedBuffer {
private:
    // max number of items in the buffer
	int cap;

    /* The queue of items in the buffer
     * Note that each item a sequence of characters that is best represented by a vector<char> for 2 reasons:
	 *  1. An STL std::string cannot keep binary/non-printables
	 *  2. The other alternative is keeping a char* for the sequence and an integer length (b/c the items can be of variable length)
	 * While the second would work, it is clearly more tedious
     */
	std::queue<std::vector<char>> q;

	// add necessary synchronization variables and data structures 
	mutex m;
	condition_variable pop_available;	// for wait by the pop, signaled by push functions
	condition_variable push_available;	// waited on by the push, signaled by pop

public:
	BoundedBuffer (int _cap);
	~BoundedBuffer ();

	void push (void* msg, int size);
	int pop (void* msg, int size);

	size_t size ();
};

#endif