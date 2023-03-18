#include "heapTimer.h"

HeapTimer::HeapTimer() {
    m_heap.reserve(c_init_reserves);
}

HeapTimer::~HeapTimer() {
    m_heap.clear();
    m_map.clear();
}

// 获取父下标
int HeapTimer::parentIndex(int i) {
    return (i - 1) / 2;
}

// 获取左孩子下标
int HeapTimer::childIndexLeft(int i) {
    return i * 2 + 1;
}

/**
 * @brief 挂载新的fd进行监听
 * 
 * @param tid fd
 * @param timeout 超时时间间隔
 * @param cb 回调
 */
void HeapTimer::add(int tid, int timeout, const TimeoutCallBack& cb) {
    assert(tid > 0);
    int i;
    
    if (m_map.count(tid) != 0) {
        // exists
        i = m_map[tid];
        m_heap[i].expire = Clock::now() + MS(timeout);
        m_heap[i].cb = cb;

        // 更新了过期时间先下沉调整，若无变化再上浮调整
        if (!__siftDown(i, m_heap.size() - 1)) __siftUp(i);
    }else {
        // no exists
        i = m_heap.size();
        m_map[tid] = i;
        m_heap.push_back({tid, Clock::now() + MS(timeout), cb });

        // 堆尾插入了元素，进行上浮调整
        __siftUp(i);
    }
}

/**
 * @brief 调整节点的过期时间
 * 
 * @param tid fd
 * @param timeout 新调整时间间隔
 */
void HeapTimer::adjust(int tid, int timeout) {
    assert(!m_heap.empty() && m_map.count(tid) > 0);

    m_heap[m_map[tid]].expire = Clock::now() + MS(timeout);
    __siftDown(m_map[tid], m_heap.size());
}

/**
 * @brief 删除节点，触发回调
 * 
 * @param tid fd
 */
void HeapTimer::drop(int tid) {
    assert(!m_heap.empty() && m_map.count(tid) != 0);

    const int i = m_map[tid];
    m_heap[i].cb();
    __del(i);
}

/**
 * @brief 清除超时节点
 */
void HeapTimer::fresh() {
    while (!m_heap.empty()) {
        TimerNode timer = m_heap.front();

        if (std::chrono::duration_cast<MS>(timer.expire - Clock::now()).count() > 0)
            break;

        timer.cb();
        __del(0);
    }
}

/**
 * @brief 获取最近节点的时间间隔
 * 
 * @return int shortest interval
 */
int HeapTimer::getNextTick() {
    fresh();
    int interval = -1;

    if (!m_heap.empty()) {
        interval = std::chrono::duration_cast<MS>(m_heap.front().expire - Clock::now()).count();
        
        if (interval < 0) interval = 0;
    }

    return interval;
}

/**
 * @brief 堆删除操作
 * 
 * @param i index
 */
void HeapTimer::__del(int i) {
    assert(!m_heap.empty() && i >= 0 && i < m_heap.size());

    const int size_ = m_heap.size();

    // 非最后个节点
    if (i < size_ - 1) {
        __swap(i, size_ - 1);

        // heapify
        if (!__siftDown(i, size_ - 1))
            __siftUp(i);
    }

    m_map.erase(m_heap.back().tid);
    m_heap.pop_back();
}

/**
 * @brief 堆化-上浮
 * 
 * @param i index
 */
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

/**
 * @brief 堆化-下沉
 * 
 * @param i index
 * @param n 边界
 * @return bool res
 */
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

/**
 * @brief 交换堆上两节点
 * 
 * @param i index1
 * @param j index2
 */
void HeapTimer::__swap(int i, int j) {
    assert(i >= 0 && i < m_heap.size());
    assert(j >= 0 && j < m_heap.size());

    std::swap(m_heap[i], m_heap[j]);
    m_map[m_heap[i].tid] = i;
    m_map[m_heap[j].tid] = j;
}
