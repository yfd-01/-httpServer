#ifndef _HEAPTIMER_H
#define _HEAPTIMER_H

#include <chrono>
#include <functional>
#include <vector>
#include <unordered_map>
#include <cassert>

typedef std::chrono::milliseconds MS;
typedef std::chrono::high_resolution_clock Clock;
typedef Clock::time_point TimePoint;
typedef std::function<void()> TimeoutCallBack;

struct TimerNode {
    int tid;
    TimePoint expire;
    TimeoutCallBack cb;

    bool operator< (const TimerNode& t) {
        return expire < t.expire;
    }
};

class HeapTimer {
public:
    HeapTimer();
    ~HeapTimer();

public:
    void add(int tid, int timeout, const TimeoutCallBack& cb);
    void drop(int tid);
    void adjust(int tid, int timeout);
    void fresh();
    int getNextTick();

    int parentIndex(int i);
    int childIndexLeft(int i);

private:
    void __siftUp(int i);
    bool __siftDown(int i, int n);
    void __del(int i);
    void __swap(int i, int j);

    std::vector<TimerNode> m_heap;
    std::unordered_map<int, int> m_map;
    const int c_init_reserves = sizeof(size_t) * 8;
};

#endif //_HEAPTIMER_H
