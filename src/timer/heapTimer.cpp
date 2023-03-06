#include "heapTimer.h"
#include <iostream>

HeapTimer::HeapTimer() {
    m_heap.reserve(c_init_reserves);
}

HeapTimer::~HeapTimer() {
    m_heap.clear();
    m_map.clear();
}

int HeapTimer::parentIndex(int i) {
    return (i - 1) / 2;
}

int HeapTimer::childIndexLeft(int i) {
    return i * 2 + 1;
}

void HeapTimer::add(int tid, int timeout, const TimeoutCallBack& cb) {
    assert(tid > 0);
    int i;
    
    if (m_map.count(tid) != 0) {
        // exists
        i = m_map[tid];
        m_heap[i].expire = Clock::now() + MS(timeout);
        m_heap[i].cb = cb;

        __siftUp(i);
    }else {
        // no exists
        i = m_heap.size();
        m_map[tid] = i;
        m_heap.push_back({tid, Clock::now() + MS(timeout), cb });

        if (!__siftDown(i, m_heap.size() - 1)) __siftUp(i);
    }
}

void HeapTimer::adjust(int tid, int timeout) {
    assert(!m_heap.empty() && m_map.count(tid) > 0);

    m_heap[m_map[tid]].expire = Clock::now() + MS(timeout);
    __siftDown(m_map[tid], m_heap.size());
}

void HeapTimer::drop(int tid) {
    assert(!m_heap.empty() && m_map.count(tid) != 0);

    const int i = m_map[tid];
    m_heap[i].cb();
    __del(i);
}

void HeapTimer::fresh() {
    while (!m_heap.empty()) {
        TimerNode timer = m_heap.front();

        if (std::chrono::duration_cast<MS>(timer.expire - Clock::now()).count() > 0)
            break;

        timer.cb();
        __del(0);
    }
}

int HeapTimer::getNextTick() {
    fresh();
    int interval = -1;

    if (!m_heap.empty()) {
        interval = std::chrono::duration_cast<MS>(m_heap.front().expire - Clock::now()).count();
        
        if (interval < 0) interval = 0;
    }

    return interval;
}

void HeapTimer::__del(int i) {
    assert(!m_heap.empty() && i >= 0 && i < m_heap.size());

    const int size_ = m_heap.size();

    // 非最后个节点
    if (i < size_ - 1) {
        __swap(i, size_ - 1);

        // heapify
        if (__siftDown(i, size_ - 1))
            __siftUp(i);
    }

    m_map.erase(m_heap.back().tid);
    m_heap.pop_back();
}

void HeapTimer::__siftUp(int i) {
    assert(i >= 0 && i < m_heap.size());
    int j = parentIndex(i);

    while (j >= 0) {
        if (m_heap[i] < m_heap[j]) {
            __swap(i, j);
            i = j;
            j = parentIndex(i);
        }else
            break;
    }
}

bool HeapTimer::__siftDown(int i, int n) {
    assert(i >= 0 && i < m_heap.size());
    assert(n >= 0 && n <= m_heap.size());

    const int init_index = i;
    int j = childIndexLeft(i);

    while (j < n) {
        if (j + 1 < n && m_heap[j + 1] < m_heap[j]) j++;
        if (m_heap[i] < m_heap[j]) break;

        __swap(i, j);
        i = j;
        j = childIndexLeft(i);
    }

    return i > init_index;
}

void HeapTimer::__swap(int i, int j) {
    assert(i >= 0 && i < m_heap.size());
    assert(j >= 0 && j < m_heap.size());

    std::swap(m_heap[i], m_heap[j]);
    m_map[m_heap[i].tid] = i;
    m_map[m_heap[j].tid] = j;
}
