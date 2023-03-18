#include "threadPool.h"

ThreadPool::ThreadPool(int thread_nums): m_thread_nums(thread_nums) {
    assert(m_thread_nums > 0);

    m_locker = std::make_shared<ThreadPoolLocker>();
    for (int i = 0; i < m_thread_nums; i++) {
        std::thread([this]{
            std::unique_lock<std::mutex> mtx(m_locker->mtx);

            while (1) {
                if (!m_tasks_queue.empty()) {
                    auto task = std::move(m_tasks_queue.front());
                    m_tasks_queue.pop();                    

                    mtx.unlock();
                    task();     // 事务任务处理
                    mtx.lock();
                }else if(m_closed) { break; }
                else {
                    m_locker->cond.wait(mtx);
                }
            }
        }).detach();
    }

    Logger::Instance()->LOG_INFO("线程池启动成功");
    m_closed = false;
}

ThreadPool::~ThreadPool() {
    m_closed = true;
    m_locker->cond.notify_all();    // 通知所有工作线程结束
}
