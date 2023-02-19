#ifndef _HEAPTIMER_H
#define _HEAPTIMER_H

#include <chrono>
#include <functional>
#include <vector>
#include <unordered_map>

typedef std::chrono::high_resolution_clock::time_point TimePoint;
typedef std::function<void()> TimeoutCallBack;

struct TimerNode {
    int tid;
    TimePoint expire;
    TimeoutCallBack cb;

    bool operator< (const TimePoint& t) {
        return expire < t.expire;
    }

    bool operator<= (const TimePoint& t) {
        return expire <= t.expire;
    }
};

class HeapTimer {
public:
    HeapTimer();
    ~HeapTimer();

public:
    void add();
    int parentIndex(int i);
    int childIndexLeft(int i);
    int childIndexRight(int i);

private:
    void __siftUp(int i);
    void __siftDown(int i);
    void __del(int i);
    void __swap(int i, int j);

    std::vector<TimePoint> m_heap;
    std::unordered_map<int, int> m_map;
    const int c_init_reserves = sizeof(size_t) * 8;
};

#endif //_HEAPTIMER_H
