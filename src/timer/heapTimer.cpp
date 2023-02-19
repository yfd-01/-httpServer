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

int HeapTimer::childIndexRight(int i) {
    return i * 2 + 2;
}

void HeapTimer::add() {

}

void HeapTimer::__del(int i) {

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

void HeapTimer::__siftDown(int i) {
    const int n = m_heap.size();
    assert(i >= 0 && i < n);

    int lc = childIndexLeft(i);
    int rc = lc + 1;

    while (lc < n) {
        bool flag1 = m_heap[i] <= m_heap[lc];
        short flag2 = rc < n ? static_cast<short>(m_heap[i] <= m_heap[rc]) : -1;

        if (flag1 && flag2)
            break;

        if (!flag1) {
            __swap(i, lc);
            i = lc;
            lc = childIndexLeft(i);
            rc = lc + 1;
        }else if (!flag2) {
            __swap(i, rc);
            i = rc;
            lc = childIndexLeft(i);
            rc = lc + 1;
        }
    }
}

void HeapTimer::__swap(int i, int j) {
    assert(i >= 0 && i < m_heap.size());
    assert(j >= 0 && j < m_heap.size());

    std::swap(m_heap[i], m_heap[j]);
    m_map[m_heap[i].tid] = i;
    m_map[m_heap[j].tid] = j;
}
