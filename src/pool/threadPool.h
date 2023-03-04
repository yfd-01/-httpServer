#ifndef _THREAD_POOL_H
#define _THREAD_POOL_H

#include <memory>
#include <functional>
#include <queue>
#include <cassert>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "../logger/logger.h"

class ThreadPool {
public:
    ThreadPool(int thread_nums);
    ~ThreadPool();

public:
    template<typename T>
    void addTask(T&& task) {
        {
            std::lock_guard<std::mutex> guard(m_locker->mtx);
            m_tasks_queue.emplace(std::forward<T>(task));
        }

        m_locker->cond.notify_one();
    }

private:
    struct ThreadPoolLocker {
        std::mutex mtx;
        std::condition_variable cond;
    };

    int m_thread_nums;
    std::shared_ptr<ThreadPoolLocker> m_locker;
    std::queue<std::function<void()>> m_tasks_queue;  // 执行任务队列
    bool m_closed;
};

#endif // _THREAD_POOL_H