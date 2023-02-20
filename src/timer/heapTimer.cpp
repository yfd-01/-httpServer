#include "heapTimer.h"

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
