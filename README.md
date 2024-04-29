# Eff_ThreadPool


## 1. 线程池的作用

1. 管理调度线程，节省线程重复申请/释放带来的损耗
2. 提高响应速度，当有任务到达时，不需要等待创建线程即可执行

## 2. 设计思路
### 2.1 分析
目前看到大多数线程池的实现方法是：提供一个接口，把任务塞进一个queue中，交给线程池中的线程异步执行，然后（不）等待结果返回。
如下图所示，m 个输入源把待执行的任务，放到一个任务队列中，然后线程池中的 n 个线程，依次去消费这个 Task queue
![simple thread pool](./Doc/image-1.png)

假设pool中一共有5个thread可以运行，而queue中有3个任务，分别是Task0，Task1，Task2。这5个thread去争抢Task0的执行权，其中有一个拿到了，去执行。然后接下来4个thread再去争抢Task1任务的执行权，一个成功之后，剩下的3个thread再去**争抢**Task2的执行权，结束。

1. 可以看到这里存在一个**争抢**，抢占锁的过程，从把任务添加到 task queue中， 以及从task queue中获取任务都是需要上锁的，这种 `one by one`的同步，相对是很耗时的，可以通过增加扇入扇出的方式来进行改进 

2. lock-free机制，内部封装`mutex`和`condition_variable`，实现AtomicQueue，AtomicPriorityQueue，LockFreeRingBufferQueue 
3. local-thread机制，把原先pool中 task queue 中的任务，放到不同的n个线程的私有的n个queue（WorkStealingQueue 类型）中，线程执行任务的时候就不需要再从pool中去**争抢**了，从而达到“增加扇出”的效果；通过写入 thread 中的 queue，达到了“增加扇入”的效果
4. work-stealing 机制，假设每个线程各自有100个任务，当任务执行时长相差较大时，thread3 的 task 都是sleep 1s，而 thread1 和 thread2 中的 task 都是sleep 1ms。正常情况下，thread仅执行自己local queue中的内容，那这个任务总体的耗时应该是max(100ms, 100ms, 100s) = 100s，thread1 和 thread2 就已经没有任务可以执行了，但是 thread3 一直到100s结束，这显然是不合理的，在 thread1 和 thread2 在执行 local task 结束之后，可以去 thread3 的队列中去stealing一些任务（就是sleep 1s的那种）执行——这就是work-stealing机制，原先100s才能执行完的任务，整体耗时瞬间就被降低到30+s左右。
5. 自动缩扩容机制，在任务繁忙的时候，pool中多加入几个thread；而在空闲的时候，对thread进行自动回收。实现中包含了两种线程：PrimaryThread（主线程）和SecondaryThread（ST, 辅助线程），默认在程序运行的时候，启动DEFAULT_THREAD_SIZE个PT去执行任务。在程序运行的过程中，主线程的数量是恒定不变的，增/减的仅可能是辅助线程。**判断忙还是空闲：**使用一个标记`running`+ `TTL（time to live）`计数的方法。除了主线程和辅助线程，pool中还开辟了一个MonitorThread（MT, 监控线程）。MT每隔固定的时间，会去轮询监测所有的主线程是否都在running状态。如果是，就认定当前pool处于忙碌状态，则添加一个ST帮忙分担任务执行。同样的，MT还会去监测每个ST的状态。如果连续TTL次监测到ST没有在执行任务，则认为pool处于空闲状态，则会销毁当前ST。
6. 任务组：适用于给一批任务，设定一个统一的等待时长/设定固定的回调函数。
7. 分支预测优化, 优化了计算路径, 实现了最优执行流程和减少循环浪费。