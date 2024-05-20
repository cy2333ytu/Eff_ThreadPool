# Eff_ThreadPool


## 1. 线程池的作用

1. 管理调度线程，节省线程重复申请/释放带来的损耗
2. 提高响应速度，当有任务到达时，不需要等待创建线程即可执行

## 2. 设计思路
### 分析
目前看到大多数线程池的实现方法是：提供一个接口，把任务塞进一个queue中，交给线程池中的线程异步执行，然后（不）等待结果返回。
如下图所示，m 个输入源把待执行的任务，放到一个任务队列中，然后线程池中的 n 个线程，依次去消费这个 Task queue
![simple thread pool](./Doc/image-1.png)

假设pool中一共有5个thread可以运行，而queue中有3个任务，分别是Task0，Task1，Task2。这5个thread去争抢Task0的执行权，其中有一个拿到了，去执行。然后接下来4个thread再去争抢Task1任务的执行权，一个成功之后，剩下的3个thread再去**争抢**Task2的执行权，结束。

1. 可以看到这里存在一个**争抢**，抢占锁的过程，从把任务添加到 task queue中， 以及从task queue中获取任务都是需要上锁的，这种 `one by one`的同步，相对是很耗时的，可以通过增加扇入扇出的方式来进行改进 

2. **lock-free机制**，内部封装`mutex`和`condition_variable`，实现AtomicQueue，AtomicPriorityQueue，LockFreeRingBufferQueue 
3. **local-thread机制**，把原先pool中 task queue 中的任务，放到不同的n个线程的私有的n个queue（WorkStealingQueue 类型）中，线程执行任务的时候就不需要再从pool中去**争抢**了，从而达到“增加扇出”的效果；通过写入 thread 中的 queue，达到了“增加扇入”的效果
4. **work-stealing 机制**，假设每个线程各自有100个任务，当任务执行时长相差较大时，thread3 的 task 都是sleep 1s，而 thread1 和 thread2 中的 task 都是sleep 1ms。正常情况下，thread仅执行自己local queue中的内容，那这个任务总体的耗时应该是max(100ms, 100ms, 100s) = 100s，thread1 和 thread2 就已经没有任务可以执行了，但是 thread3 一直到100s结束，这显然是不合理的，在 thread1 和 thread2 在执行 local task 结束之后，可以去 thread3 的队列中去stealing一些任务（就是sleep 1s的那种）执行——这就是work-stealing机制，原先100s才能执行完的任务，整体耗时瞬间就被降低到30+s左右。
5. **自动缩扩容机制**，在任务繁忙的时候，pool中多加入几个thread；而在空闲的时候，对thread进行自动回收。实现中包含了两种线程：PrimaryThread（主线程）和SecondaryThread（ST, 辅助线程），默认在程序运行的时候，启动DEFAULT_THREAD_SIZE个PT去执行任务。在程序运行的过程中，主线程的数量是恒定不变的，增/减的仅可能是辅助线程。**判断忙还是空闲**:使用一个标记`running`+ `TTL（time to live）`计数的方法。除了主线程和辅助线程，pool中还开辟了一个MonitorThread（MT, 监控线程）。MT每隔固定的时间，会去轮询监测所有的主线程是否都在running状态。如果是，就认定当前pool处于忙碌状态，则添加一个ST帮忙分担任务执行。同样的，MT还会去监测每个ST的状态。如果连续TTL次监测到ST没有在执行任务，则认为pool处于空闲状态，则会销毁当前ST。
6. **任务组**：适用于给一批任务，设定一个统一的等待时长/设定固定的回调函数。
7. **分支预测优化**： 优化了计算路径, 实现了最优执行流程和减少循环浪费。

## 3. Benchmark
    使用的测试设备配置如下
    Ubuntu虚拟机：16核, 8GB运行内存

分别对 `EFF_ThreadPool` 以及 常规的 `ThreadPool` 进行了测试，使用16线程，对 10000, 10.0000, 20.0000, 40.0000, 80.0000, 100.0000, 500.0000 个空 tasks 进行调度。

下图是对 `EFF_ThreadPool`进行测试的结果
![EFF_ThreadPool](./Doc/benchmarkTestEFF.png)


下图是对 常规ThreadPool 进行测试的结果
![ThreadPool](./Doc/benchmarkTestOther.png)


使用最大的任务数量为 500.0000 任务的数据来计算性能

**EFF_ThreadPool**
总时间：4.871秒
任务数：5000000
每秒处理的任务数量 = 任务数 / 总时间 = 5000000 / 4.871 ≈ 1026323.918任务/秒


常规ThreadPool
总时间：796557秒
任务数：5000000
每秒处理的任务数量 = 任务数 / 总时间 = 5000000 / 796557 ≈ 6.277任务/秒

EFF_ThreadPool 每秒能达到 **百万并发**，是 常规ThreadPool 的 1026323.918 / 6.277 ≈ **163491.889倍**


# Eff_ThreadPool

## 1. Purpose of the Thread Pool

1. Manages and schedules threads, saving the overhead caused by repeatedly creating and releasing threads.
2. Improves response speed as tasks can be executed immediately upon arrival without waiting for thread creation.

## 2. Design Philosophy
### Analysis
The common implementation of thread pools involves providing an interface to push tasks into a queue, which are then asynchronously executed by threads in the pool, followed by waiting (or not) for the result to return.
As shown in the diagram below, m input sources place tasks to be executed into a task queue, which are then consumed by n threads within the thread pool.
![simple thread pool](./Doc/image-1.png)

Assuming there are 5 threads available in the pool and 3 tasks in the queue, namely Task0, Task1, and Task2. These 5 threads compete for the right to execute Task0, and once one succeeds, the remaining 4 threads compete for Task1, and then the remaining 3 for Task2, and so on.

1. One can observe a **competition** for lock acquisition throughout the process, from adding tasks to the task queue and retrieving tasks from it. This `one by one` synchronization is relatively time-consuming and could be improved by increasing the fan-in and fan-out.
   
2. **Lock-free mechanism**: Internally encapsulates `mutex` and `condition_variable` to implement AtomicQueue, AtomicPriorityQueue, LockFreeRingBufferQueue.

3. **Local-thread mechanism**: Moves tasks from the pool's main task queue into private queues of the different threads (WorkStealingQueue type). This means threads no longer need to compete for tasks, effectively increasing the fan-out. Writing tasks into the thread's queue increases the fan-in.

4. **Work-stealing mechanism**: Suppose each thread has 100 tasks, with varying execution times. For instance, tasks in thread3 sleep for 1s, while those in thread1 and thread2 sleep for 1ms. Normally, a thread only executes tasks from its local queue. However, once thread1 and thread2 finish their tasks, they can "steal" tasks from thread3's queue (the 1s sleep tasks) to execute. This significantly reduces the total task completion time from 100s to around 30+s.

5. **Dynamic scaling mechanism**: During busy periods, more threads are added to the pool, and during idle times, threads are automatically reclaimed. The pool contains PrimaryThreads (PT) and SecondaryThreads (ST), with a fixed number of PTs operational at start-up and STs adjusted according to workload. **Determining busyness or idleness**: Uses a ‘running’ flag in combination with a ‘TTL (time to live)’ count. In addition to PTs and STs, a MonitorThread (MT) is deployed to periodically check the running status of PTs. If all are busy, an ST is added to help with task execution. Similarly, MT also checks the status of each ST, removing those inactive for consecutive TTL cycles.

6. **Task groups**: Suitable for assigning a unified wait time or callback function to a batch of tasks.

7. **Branch prediction optimization**: Optimized execution paths reducing loop overhead and achieving more efficient execution.

## 3. Benchmark
Testing was conducted on the following setup: 
Ubuntu VM: 16 cores, 8GB RAM

Both `EFF_ThreadPool` and a conventional `ThreadPool` were tested using 16 threads to schedule 10,000 to 5,000,000 empty tasks.

Below are the results for testing `EFF_ThreadPool`:
![EFF_ThreadPool](./Doc/benchmarkTestEFF.png)

Below are the results for the conventional ThreadPool:
![ThreadPool](./Doc/benchmarkTestOther.png)

Using the largest task number (5,000,000) to calculate performance:

**EFF_ThreadPool**
Total time: 4.871 seconds
Number of tasks: 5,000,000
Tasks processed per second = Number of tasks / Total time = 5,000,000 / 4.871 ≈ 1,026,323.918 tasks/second

**Conventional ThreadPool**
Total time: 796557 seconds
Number of tasks: 5,000,000
Tasks processed per second = Number of tasks / Total time = 5,000,000 / 796557 ≈ 6.277 tasks/second

EFF_ThreadPool can achieve **millions of concurrent operations per second**, which is roughly **163491.889 times more efficient** than the conventional ThreadPool.
