#ifndef _BLOCKING_DEQUE_H
#define _BLOCKING_DEQUE_H

#include <cassert>
#include <chrono>
#include <deque>
#include <memory>
#include <condition_variable>
#include <mutex>


template <class T>
class BlockingDeque {
public:
    BlockingDeque(int capacity);
    ~BlockingDeque();

public:
    int size();
    int capacity();
    bool empty();
    bool full();

    T& front();
    T& back();
    void pushFront(const T& t);
    bool pop(T& item);
    bool pop(T& item, int timeout);
    void flush();
    void clear();

private:
    std::deque<T> m_deq;
    int m_capacity;
    bool m_closed;

    struct BlockingLocker {
        std::mutex mtx;
        std::condition_variable producerCond;
        std::condition_variable consumerCond;
    };

    BlockingLocker* m_blockingLocker;
};

template<class T>
BlockingDeque<T>::BlockingDeque(int capacity): m_capacity(capacity) {
    assert(capacity > 0);

    m_blockingLocker = new BlockingLocker();
    m_closed = false;
}

template<class T>
BlockingDeque<T>::~BlockingDeque() {
    clear();
}

template<class T>
void BlockingDeque<T>::clear() {
    {
        std::lock_guard<std::mutex> locker(m_blockingLocker->mtx);
        m_deq.clear();
        m_closed = true;
    }

    m_blockingLocker->producerCond.notify_all();    // 先将close标志置为true，再唤醒消费者锁从pop函数中为假返回，使得logger所执行的线程
    m_blockingLocker->consumerCond.notify_all();    // 跳出，该阻塞线程结束后使得主线程得及结束
}


template<class T>
int BlockingDeque<T>::capacity() { return m_capacity; }

template<class T>
int BlockingDeque<T>::size() { 
    std::lock_guard<std::mutex> locker(m_blockingLocker->mtx);
    return m_deq.size(); 
}

template<class T>
bool BlockingDeque<T>::empty() {
    std::lock_guard<std::mutex> locker(m_blockingLocker->mtx);
    return m_deq.empty();
}

template<class T>
bool BlockingDeque<T>::full(){
    std::lock_guard<std::mutex> locker(m_blockingLocker->mtx);
    return m_deq.size() >= m_capacity;
}


template<class T>
T& BlockingDeque<T>::front() {
    std::lock_guard<std::mutex> locker(m_blockingLocker->mtx);
    return m_deq.front();
}

template<class T>
T& BlockingDeque<T>::back() {
    std::lock_guard<std::mutex> locker(m_blockingLocker->mtx);
    return m_deq.back();
}

template<class T>
void BlockingDeque<T>::pushFront(const T& t) {
    std::unique_lock<std::mutex> locker(m_blockingLocker->mtx);
    
    while (m_deq.size() >= m_capacity)
        m_blockingLocker->producerCond.wait(locker);

    m_deq.push_front(t);
    m_blockingLocker->consumerCond.notify_one();
}

template<class T>
bool BlockingDeque<T>::pop(T& item) {
    std::unique_lock<std::mutex> locker(m_blockingLocker->mtx);

    while (m_deq.empty()) {
        if (m_closed)
            return false;

        m_blockingLocker->consumerCond.wait(locker);
    }

    item = m_deq.back();
    m_deq.pop_back();

    m_blockingLocker->producerCond.notify_one();

    return true;
}

template<class T>
bool BlockingDeque<T>::pop(T& item, int timeout) {
    std::unique_lock<std::mutex> locker(m_blockingLocker->mtx);

    while (m_deq.empty()) {
        if (m_closed)
            return false;

        if (m_blockingLocker->consumerCond.wait_for(locker, std::chrono::milliseconds(timeout))
            == std::cv_status::timeout) {
            return false;
        }
    }

    item = m_deq.back();
    m_deq.pop_back();

    m_blockingLocker->producerCond.notify_one();
    
    return true;
}

template<class T>
void BlockingDeque<T>::flush() {
    m_blockingLocker->consumerCond.notify_one();
}

#endif // _BLOCKING_DEQUE_H
