#include "Queue/AtomicPriorityQueue.h"
#include "Queue/AtomicQueue.h"
#include "Queue/WorkStealingQueue.h"
#include <cassert>
#include <string>
#include <thread>
#include <iostream>
#include <atomic>
ccy::AtomicPriorityQueue<std::string> queuePrio;
// ccy::AtomicQueue<int> queue;
ccy::WorkStealingQueue<int> queue;

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

void test_single_thread_push_pop() {
    ccy::WorkStealingQueue<int> queue;
    
    // 测试单线程push和pop
    queue.push(1);
    queue.push(2);
    
    int value;
    assert(queue.tryPop(value) && value == 2); // pop应该返回2
    assert(queue.tryPop(value) && value == 1); // pop应该返回1
    assert(!queue.tryPop(value)); // 队列已空

    std::cout << "test_single_thread_push_pop passed" << std::endl;
}

void test_single_thread_push_try_steal() {
    ccy::WorkStealingQueue<int> queue;
    
    // 测试单线程push和trySteal
    queue.push(1);
    queue.push(2);
    
    int value;
    assert(queue.trySteal(value) && value == 1); // trySteal应该返回1
    assert(queue.trySteal(value) && value == 2); // trySteal应该返回2
    assert(!queue.trySteal(value)); // 队列已空

    std::cout << "test_single_thread_push_try_steal passed" << std::endl;
}

void test_multiple_thread_push_pop() {
    ccy::WorkStealingQueue<int> queue;
    std::vector<std::thread> threads;
    const int num_threads = 10;
    const int num_pushes_per_thread = 1000;

    // 多线程同时push
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&queue, i, num_pushes_per_thread]() {
            for (int j = 0; j < num_pushes_per_thread; ++j) {
                queue.push(i * num_pushes_per_thread + j);
            }
        });
    }

    // 等待所有push完成
    for (auto& t : threads) {
        t.join();
    }

    // assert(queue.size() == num_threads * num_pushes_per_thread); // 确保所有元素都push了进去

    // 多线程同时pop
    std::atomic<int> pop_count{0};
    threads.clear();

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&queue, &pop_count]() {
            int value;
            while (queue.tryPop(value)) {
                ++pop_count;
            }
        });
    }

    // 等待所有pop完成
    for (auto& t : threads) {
        t.join();
    }

    assert(pop_count == num_threads * num_pushes_per_thread); // 确保所有元素都被pop了出来
    std::cout << "test_multiple_thread_push_pop passed" << std::endl;
}

void testWorkStealing(){
    test_single_thread_push_pop();
    test_single_thread_push_try_steal();
    test_multiple_thread_push_pop();

    // 这里可以加更多的测试用例，比如测试tryPush, tryPop, 带有优先级的批量pop等等

    std::cout << "All tests passed" << std::endl;
    
}

int main() {
    // testPriorityQueue();
    // testQueue();
    testWorkStealing();
    return 0;
}
