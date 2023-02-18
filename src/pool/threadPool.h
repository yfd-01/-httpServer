#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <cassert>
#include <memory>

struct ThreadPoolLocker {
    std::mutex mtx;
    std::condition_variable cond;
};


class ThreadPool {
public:
    ThreadPool(int thread_nums);
    ~ThreadPool();

public:
    template<class T>
    void addTask(T&& task);

private:
    int m_thread_nums;
    std::shared_ptr<ThreadPoolLocker> m_locker;
    std::queue<void (*)(void)> m_tasks_queue;  // 执行任务队列
    bool m_closed;
};

#endif // _THREAD_POOL_H