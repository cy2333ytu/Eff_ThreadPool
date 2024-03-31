#include "Queue/AtomicPriorityQueue.h"
#include "Queue/AtomicQueue.h"
#include <cassert>
#include <string>
#include <thread>
#include <iostream>

ccy::AtomicPriorityQueue<std::string> queuePrio;
ccy::AtomicQueue<int> queue;
void testPriorityQueue(){
    assert(queuePrio.empty() == true);
    std::string item;
    assert(queuePrio.tryPop(item) == false);

    queuePrio.push("item_1", 1);
    assert(queuePrio.empty() == false);
    // std::cout << item << '\n';
    assert(queuePrio.tryPop(item) == true && item != "Item_1");
    std::cout << "-------------" << '\n';
    std::cout<< item << "\n"; 
    std::cout << "-------------" << '\n';

    std::cout << "All PriorityQueue tests passed successfully!\n";

}

void testWaitPop(ccy::AtomicQueue<int>& queue) {
    int value;
    queue.waitPop(value);
    std::cout << "waitPop: Got value " << value << std::endl;
}

void testTryPop(ccy::AtomicQueue<int>& queue) {
    int value;
    if (queue.tryPop(value)) {
        std::cout << "tryPop: Got value " << value << std::endl;
    } else {
        std::cout << "tryPop: No value to pop" << std::endl;
    }
}

void testTryPopBatch(ccy::AtomicQueue<int>& queue) {
    std::vector<int> values;
    if (queue.tryPop(values, 5)) {
        std::cout << "tryPopBatch: Popped " << values.size() << " values" << std::endl;
    } else {
        std::cout << "tryPopBatch: No values to pop" << std::endl;
    }
}

void testPopWithTimeout(ccy::AtomicQueue<int>& queue) {
    auto value = queue.popWithTimeout(1000); // 1 second timeout
    if (value) {
        std::cout << "popWithTimeout: Got value " << *value << std::endl;
    } else {
        std::cout << "popWithTimeout: Timeout without popping value" << std::endl;
    }
}

void testTryPopNonBlocking(ccy::AtomicQueue<int>& queue) {
    auto value = queue.tryPop();
    if (value) {
        std::cout << "tryPopNonBlocking: Got value " << *value << std::endl;
    } else {
        std::cout << "tryPopNonBlocking: No value to pop" << std::endl;
    }
}

void testPush(ccy::AtomicQueue<int>& queue) {
    queue.push(42);
    std::cout << "Pushed value 42" << std::endl;
}

void testEmpty(ccy::AtomicQueue<int>& queue) {
    bool isEmpty = queue.empty();
    std::cout << "Queue is " << (isEmpty ? "empty" : "not empty") << std::endl;
}

void testQueue() {
    // Create a new queue for testing
    ccy::AtomicQueue<int> queue;

    // Test the empty state before any operations
    testEmpty(queue);

    // Test pushing elements onto the queue
    testPush(queue);

    // Test waitPop function
    testWaitPop(queue);

    // Queue should now be empty again
    testEmpty(queue);

    // Test the tryPop function when queue is empty
    testTryPop(queue);

    // Test the tryPopBatch function when queue is empty
    testTryPopBatch(queue);

    // Test the popWithTimeout function when queue is empty
    testPopWithTimeout(queue);

    // Fill the queue with multiple items for batch operations
    for (int i = 0; i < 10; ++i) {
        queue.push(std::move(i));
    }

    // Test the batch pop function with items
    testTryPopBatch(queue);

    // Add a single item and test nonblocking pop
    queue.push(99);
    testTryPopNonBlocking(queue);

    // Anything left should cause the queue to be non-empty
    testEmpty(queue);
}

int main() {
    // testPriorityQueue();
    testQueue();
    return 0;
}
