# Eff_ThreadPool


## 1. 线程池的作用

1. 管理调度线程，节省线程重复申请/释放带来的损耗
2. 提高响应速度，当有任务到达时，不需要等待创建线程即可执行

## 2. 设计思路
目前看到的数线程池的实现方法是：提供一个接口，把任务塞进一个queue中，交给线程池中的线程异步执行，然后（不）等待结果返回

![simple thread pool](/Eff_ThreadPool/Doc/image-1.png)
如上图所示，m 个输入源把待执行的任务，放到一个任务队列中，然后线程池中的 n 个线程，依次去消费这个 Task queue
